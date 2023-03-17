import React from 'react';
import { withStyles } from 'tss-react/mui';
import Paper from '@mui/material/Paper';

import FormControl from '@mui/material/FormControl';
import MenuItem from '@mui/material/MenuItem';
import Select from '@mui/material/Select';
import InputLabel from '@mui/material/InputLabel';

import AppBar from '@mui/material/AppBar';
import Tabs from '@mui/material/Tabs';
import Tab from '@mui/material/Tab';
import ModeList from '../component/Scheduler/ModeList';
import InfoList from '../component/Scheduler/InfoList';
import VariableList from '../component/Scheduler/VariableList'
import { STYLES_SCHEDULER_CONFIGURATION } from '../styles';

import Button from '@mui/material/Button';
import IconButton from '@mui/material/IconButton';

import Box from '@mui/material/Box';

import CheckIcon from '@mui/icons-material/Check';
import CloseIcon from '@mui/icons-material/Close';
import LockIcon from '@mui/icons-material/Lock';
import LockOpenIcon from '@mui/icons-material/LockOpen';

import {
  getDataForURI, setOrPutDataForURI
} from '../../AppConfig';

import { socket_scheduler } from '../../AppAuth';
import { FormHelperText } from '@mui/material';

import LoadingHOC from '../component/LoadingHOC';

const LIGHT_GRAY = '#D1D1D1';

function TabPanel(props) {
  const {
    children,
    value,
    index,
  } = props;

  return (
    <div style={{ width: '100%', height: '100%' }}
      hidden={value !== index}
    >
      {value === index && (
        <div style={{ width: '100%', height: '100%' }}>
          {children}
        </div>
      )}
    </div>
  );
}


class SchedulerConfig extends React.PureComponent {
  constructor(props) {
    super(props);
    this.state = {
      selectedMode: null,
      selectedModeWithDeletions: null,
      selectedSiteWithUpdates: null,
      updatingMode: null,
      modeData: [],
      siteData: null,
      connection: "",
      tabIndex: 0,
      showConfirmation: false,
      socket: socket_scheduler,
      buttonType: '',
      errors: 0,
      isLoadingSiteData: true,
      isLoadingModeData: true,
      locked: true,
    };
    this.props.setLoading(true)
    this.selectMode = this.selectMode.bind(this);
    this.submitNewMode = this.submitNewMode.bind(this);
    this.submitNewSite = this.submitNewSite.bind(this);
    this.updateUpdatingSite = this.updateUpdatingSite.bind(this);
    this.handleTabsChange = this.handleTabsChange.bind(this);
    this.confirmDeleteButtonPressed = this.confirmDeleteButtonPressed.bind(this);
    this.updateUpdatingModeWith = this.updateUpdatingModeWith.bind(this);
    this.isNewNameValid = this.isNewNameValid.bind(this);
    this.isNewIdValid=this.isNewIdValid.bind(this)
    this.handleVariableListError = this.handleVariableListError.bind(this);
    this.handleInfoListError = this.handleInfoListError.bind(this);

    this._isMounted = false;

  }

  /**
   * Sets up sockets and fetches data
   */
  componentDidMount() {
    this._isMounted = true;
    this.state.socket.on('/scheduler/configuration', (data) => {
      this.updateSiteData(JSON.parse(data));
    })

    this.state.socket.on('/scheduler/modes', (data) => {
      this.updateModeStateData(JSON.parse(data));
    });
    

    setTimeout(() => {
      this._isMounted && this.fetchData();
    }, 1000);
  }

  /**
   * Closes sockets
   */
  componentWillUnmount() {
    clearInterval()
    this.state.socket.off('/scheduler/modes');
    this.state.socket.off('/scheduler/configuration');
    this._isMounted = false;
  }

  /**
   * Fetches modes and configuration (site controller VS FM)
   */
  fetchData() {
    getDataForURI('scheduler/modes')
      .then((response) => {
        if (response.ok) {

          return response.json();

        }
        throw new Error(`${response.statusText} : Cannot get SCHEDULER MODES`);
      })
      .catch((error) => this.setState({ error }));

    getDataForURI('scheduler/configuration')
      .then((response) => {
        if (response.ok) {
          return response.json();

        }
        throw new Error(`${response.statusText} : Cannot get SCHEDULER MODES`);
      })
      .catch((error) => this.setState({ error }));

  }

  /**
   * Gets site data (siteIDs, names, etc.) and the connection type (FM or SC)
   */
  updateSiteData(data) {
    this.setState({
      siteData: data,
      connection: data['connection'] != null ? data['connection'] : "",
      isLoadingSiteData: false
    }, () => {if (!this.state.isLoadingModeData) {
      this.props.setLoading(false)
    }})
  }

  /**
   * Sets up mode data
   */
  updateModeStateData(data) {
    this.setState({
      modeData: data.modes,
      isLoadingModeData: false
    }, () => {if (!this.state.isLoadingSiteData) {
      this.props.setLoading(false)
    }})
  }

  /**
   * When an error is made or fixed, this is called to keep track of errors to avoid submitting bad data
   * @param {number} error - either +1 or -1
   */
  handleVariableListError(error) {
    this.setState({
      errors: this.state.errors + error
    });
  }

  /**
   * When an error is made or fixed, this is called to keep track of errors to avoid submitting bad data
   * @param {number} error - either +1 or -1
   */
  handleInfoListError(error) {
    this.setState({
      errors: this.state.errors + error
    });
  }

  /**
   * Called when creating new mode (just has modeTitle with no vars or constants) or submitting edited mode
   * @param {object} newMode - mode object
   */
  submitNewMode(newMode) {
    let newTitle = Object.keys(newMode)[0];
    this.setState({
      modeData: {
        ...this.state.modeData,
        [newTitle]: newMode[newTitle],
      },
      selectedMode: newMode,
      locked: true,
      selectedModeWithDeletions: JSON.parse(JSON.stringify(newMode)),
      updatingMode: JSON.parse(JSON.stringify(newMode))
    }, () => this.sendModesToBackEnd());
  }

  /**
   * Called when new site is created (either gets added to a list in the case of FM or is just the 1 site for SC)
   * @param {object} newSite - siteData (ID, name, port number, IP)
   */
  submitNewSite(newSite) {
    if (this.state.connection == 'FM') {
      let newData =  JSON.parse(JSON.stringify(this.state.siteData));
      newData['sites'].push(newSite);
      newData['sites'].sort((a, b) => (a.siteName > b.siteName) ? 1 : -1)
      this.setState({siteData: newData, locked: true}, () => this.sendConfigurationToBackEnd())
    } else if (this.state.connection == "SC" && newSite['siteControllerPort'] != null) {
      let newData = {
        "connection" : "SC",
        "siteName" : newSite['siteName'],
        "siteId" : newSite['siteId'],
        "siteControllerPort" : newSite['siteControllerPort']
      }
      this.setState({siteData: newData, locked: true}, () => this.sendConfigurationToBackEnd())
    } else {
      let newData = {
        "connection" : "SC",
        "siteName" : newSite['siteName'],
        "siteId" : newSite['siteId'],
      }
      this.setState({siteData: newData, locked: true}, () => this.sendConfigurationToBackEnd())
    }
  }

  /**
   * Selects either the mode or the site
   * @param {object} modeData - either the mode or site data being selected
   * @param {boolean} isMode - true when selecting a mode, false for site
   */
  selectMode(modeData, isMode=true) {
    let newSiteDataCopy = null;
    if (!isMode) {
      modeData = {
        "siteName" : modeData['siteName']
      };
      if (this.state.siteData.connection == "SC" && modeData['siteControllerPort'] != "") {
        modeData['siteControllerPort'] = this.state.siteData.siteControllerPort;
      }
      if (this.state.siteData.connection == "SC") {
        modeData['siteId'] = this.state.siteData.siteId
      }
      if (this.state.siteData.connection == "FM") {
        let theSite = -1;
        for (let i = 0; i < this.state.siteData.sites.length; i++) {
          if (this.state.siteData.sites[i].siteName == modeData['siteName']) {
            theSite = i;
            break;
          }
        }
        modeData['siteControllerPort'] = this.state.siteData.sites[theSite].siteControllerPort;
        modeData['siteControllerIP'] = this.state.siteData.sites[theSite].siteControllerIP;
        modeData['siteId'] = this.state.siteData.sites[theSite].siteId;
      }
      newSiteDataCopy = JSON.parse(JSON.stringify(modeData))
    }
    this.setState({
      showConfirmation: false,
      selectedMode: newSiteDataCopy == null ? modeData : JSON.parse(JSON.stringify(newSiteDataCopy)),
      selectedModeWithDeletions: JSON.parse(JSON.stringify(modeData)),
      updatingMode: JSON.parse(JSON.stringify(modeData)),
      selectedSiteWithUpdates: newSiteDataCopy
    })
  }

  /**
   * For modes, there are different tabs for info, vars, and constants, and this manages it
   */
  handleTabsChange(event, tabIndex) {
    this.setState({
      tabIndex: tabIndex
    });
  }

  /**
   * Returns true if neither a site or mode is selected
   */
  editingIsDisabled() {
    return (this.state.selectedMode == null);
  }

  /**
   * As the mode is being edited, any changes will be saved to a new JSON object that can be either discarded or saved when editing is complete
   * @param {object} newSelectedModeWithDeletions - this will be the nonedited mode but any deletions are included (helps with equality checking)
   */
  updateUpdatingModeWith(newMode, newSelectedModeWithDeletions = null) {
    if (newSelectedModeWithDeletions != null) {
      this.setState({
        updatingMode: newMode,
        selectedModeWithDeletions: newSelectedModeWithDeletions
      });
    }
    this.setState({
      updatingMode: newMode
    });
  }

  /**
   * Deletes either mode or site depending on what is selected
   */
  confirmDeleteButtonPressed() {
    if (!Object.keys(this.state.selectedMode).includes('siteName')) {
      let modeDataCopy = JSON.parse(JSON.stringify(this.state.modeData));
      let index = -1;
      let modeKeys = Object.keys(modeDataCopy);
      let target = Object.keys(this.state.selectedMode)[0];
      delete modeDataCopy[target];

      this.reset(modeDataCopy);
    } else {
      if (this.state.siteData.connection == 'FM') {
        let siteDataCopy = JSON.parse(JSON.stringify(this.state.siteData.sites));
        let num = 0;
        for (let i = 0; i < siteDataCopy.length; i++) {
          if (siteDataCopy[i].siteName == this.state.selectedMode.siteName) {
            num = i;
            break;
          }
        }
        delete siteDataCopy[num];
        siteDataCopy.splice(num, 1)
        let siteDataFullCopy = JSON.parse(JSON.stringify(this.state.siteData));
        siteDataFullCopy.sites = siteDataCopy;
        this.reset(siteDataFullCopy);
      }
    }
    
  }

  /**
   * Opens up confirmation
   * @param {string} type - either cancel or save
   */
  buttonPressed(type) {
    this.setState({
      showConfirmation: true,
      buttonType: type
    });
  }

  /**
   * Either cancels or saves depending on what buttonType was selected (save or cancel)
   */
  confirmButton() {
    let modeDataCopy = JSON.parse(JSON.stringify(this.state.modeData));
    if (this.state.buttonType === 'save') {
      if (!Object.keys(this.state.selectedMode).includes('siteName')) {
        let index = -1;
        let key = Object.keys(this.state.selectedMode)[0];
        delete modeDataCopy[key];
        let newKey = Object.keys(this.state.updatingMode)[0]
        modeDataCopy[newKey] = this.state.updatingMode[newKey];
        this.reset(modeDataCopy);
      } else {
        let updatedSite = this.state.selectedSiteWithUpdates
        if (this.state.connection == 'FM') {
          let newData =  JSON.parse(JSON.stringify(this.state.siteData));
          let replaceHere = -1;
          for (let i = 0; i < newData.sites.length; i++) {
            if (newData.sites[i].siteName == this.state.selectedMode.siteName) {
              replaceHere = i;
              break;
            }
          }
          newData.sites[replaceHere].siteName = updatedSite.siteName;
          newData.sites[replaceHere].siteId = updatedSite.siteId;
          newData.sites[replaceHere].siteControllerIP = updatedSite.siteControllerIP;
          newData.sites[replaceHere].siteControllerPort = updatedSite.siteControllerPort;
         
          this.setState({siteData: newData, locked: true}, () => {this.sendConfigurationToBackEnd(); this.reset();})
        } else if (this.state.connection == "SC" && updatedSite.siteControllerPort != "") {
          let newData =  JSON.parse(JSON.stringify(this.state.siteData));
          newData.siteName = updatedSite.siteName;
          newData.siteId = updatedSite.siteId;
          newData.siteControllerPort = updatedSite.siteControllerPort;
          this.setState({siteData: newData, locked: true}, () => {this.sendConfigurationToBackEnd(); this.reset()})
        } else if (this.state.connection == "SC" && updatedSite.siteControllerPort == "") {
          let newData =  {};
          newData.siteName = updatedSite.siteName;
          newData.siteId = updatedSite.siteId;

          newData.connection = "SC";
          
          this.setState({siteData: newData, locked: true}, () => {this.sendConfigurationToBackEnd(); this.reset()})
        }
      }
      
    } else {
      this.reset(modeDataCopy);
    }

  }

  cancelButton() {
    this.setState({
      showConfirmation: false,
      buttonType: ''
    })
  }

  dismissChangesButtonPressed() {

    this.setState({
      updatingMode: this.state.selectedMode,
    }, () => this.reset())
  }

  reset(newModeData = null) {
    if (newModeData == null) {
      this.setState({
        selectedMode: null,
        selectedSiteWithUpdates: null,
        selectedModeWithDeletions: null,
        updatingMode: null,
        tabIndex: 0,
        showConfirmation: false,
        errors: 0,
        locked: true
      });
    } else {
      let updated = "modeData"
      if (Object.keys(newModeData).includes('connection')) {
        this.setState({
          siteData: newModeData,
          selectedMode: null,
          selectedSiteWithUpdates: null,
          updatingMode: null,
          selectedModeWithDeletions: null,
          tabIndex: 0,
          showConfirmation: false,
          errors: 0,
          locked: true
        }, () => {
          if (!Object.keys(newModeData).includes('connection')) {
            this.sendModesToBackEnd();
          } else {
            this.sendConfigurationToBackEnd()
          }
          
        });
      } else {
        this.setState({
          modeData: newModeData,
          selectedMode: null,
          selectedSiteWithUpdates: null,
          updatingMode: null,
          selectedModeWithDeletions: null,
          tabIndex: 0,
          showConfirmation: false,
          errors: 0,
          locked: true
        }, () => {
          if (!Object.keys(newModeData).includes('connection')) {
            this.sendModesToBackEnd();
          } else {
            this.sendConfigurationToBackEnd()
          }
          
        });
      }
      // this.setState({
      //   modeData: newModeData,
      //   selectedMode: null,
      //   updatingMode: null,
      //   selectedModeWithDeletions: null,
      //   tabIndex: 0,
      //   showConfirmation: false,
      //   errors: 0
      // }, () => {
      //   if (!Object.keys(newModeData).includes('connection')) {
      //     this.sendModesToBackEnd();
      //   } else {
      //     this.sendConfigurationToBackEnd()
      //   }
        
      // });
    }

  }

  sendConfigurationToBackEnd() {
    setOrPutDataForURI('scheduler/configuration', JSON.stringify(this.state.siteData), 'POST')
      .then((response) => {
        if (response.ok) {
          return response.json();
        }
        throw new Error(`${response.statusText} : Cannot updatePropertyValue() for: ${response.url}`);
      })
      .catch((error) => {
        throw new Error(`InputFieldControl/doUpdatePropertyValue error: ${error}`);
      });
  }

  sendModesToBackEnd() {
    let sentBackObject = {};
    sentBackObject['modes'] = this.state.modeData
    let differentModes = Object.keys(sentBackObject['modes']);
    for (let i = 0; i < differentModes.length; i++) {
      for (let j = 0; j < sentBackObject['modes'][differentModes[i]]['constants'].length; j++) {
        if (sentBackObject['modes'][differentModes[i]]['constants'][j]['type'] == 'Bool') {
            if (typeof sentBackObject['modes'][differentModes[i]]['constants'][j]['value'] == 'string' && sentBackObject['modes'][differentModes[i]]['constants'][j]['value'].toLowerCase() == 'true') {
              sentBackObject['modes'][differentModes[i]]['constants'][j]['value'] = true;
            } else {
              sentBackObject['modes'][differentModes[i]]['constants'][j]['value'] = sentBackObject['modes'][differentModes[i]]['constants'][j]['value'] == 'true' || sentBackObject['modes'][differentModes[i]]['constants'][j]['value'] == true ? true: false;
            }
        } else if (sentBackObject['modes'][differentModes[i]]['constants'][j]['type'] == 'Int') {
            sentBackObject['modes'][differentModes[i]]['constants'][j]['value'] = parseInt(sentBackObject['modes'][differentModes[i]]['constants'][j]['value']);
        } else if  (sentBackObject['modes'][differentModes[i]]['constants'][j]['type'] == 'Float') {
            sentBackObject['modes'][differentModes[i]]['constants'][j]['value'] = parseFloat(sentBackObject['modes'][differentModes[i]]['constants'][j]['value']);
        }
      }

      for (let j = 0; j < sentBackObject['modes'][differentModes[i]]['variables'].length; j++) {
        if (sentBackObject['modes'][differentModes[i]]['variables'][j]['type'] == 'Bool') {
            if (typeof sentBackObject['modes'][differentModes[i]]['variables'][j]['value'] == 'string' && sentBackObject['modes'][differentModes[i]]['variables'][j]['value'].toLowerCase() == 'true') {
              sentBackObject['modes'][differentModes[i]]['variables'][j]['value'] = true;
            } else {
              sentBackObject['modes'][differentModes[i]]['variables'][j]['value'] = sentBackObject['modes'][differentModes[i]]['variables'][j]['value'] == 'true' || sentBackObject['modes'][differentModes[i]]['variables'][j]['value'] == true ? true: false;
            }
        } else if (sentBackObject['modes'][differentModes[i]]['variables'][j]['type'] == 'Int') {
            sentBackObject['modes'][differentModes[i]]['variables'][j]['value'] = parseInt(sentBackObject['modes'][differentModes[i]]['variables'][j]['value']);
            if (isNaN(sentBackObject['modes'][differentModes[i]]['variables'][j]['value'])) {
              sentBackObject['modes'][differentModes[i]]['variables'][j]['value'] = 0;
            }
        } else if  (sentBackObject['modes'][differentModes[i]]['variables'][j]['type'] == 'Float') {
            sentBackObject['modes'][differentModes[i]]['variables'][j]['value'] = parseFloat(sentBackObject['modes'][differentModes[i]]['variables'][j]['value']);
            if (isNaN(sentBackObject['modes'][differentModes[i]]['variables'][j]['value'])) {
              sentBackObject['modes'][differentModes[i]]['variables'][j]['value'] = 0.0;
            }
        }
      }
    }
    setOrPutDataForURI('scheduler/modes', JSON.stringify(sentBackObject), 'POST')
      .then((response) => {
        if (response.ok) {
          return response.json();
        }
        throw new Error(`${response.statusText} : Cannot updatePropertyValue() for: ${response.url}`);
      })
      .catch((error) => {
        throw new Error(`InputFieldControl/doUpdatePropertyValue error: ${error}`);
      });
  }

  isNewNameValid(offeredName, originalName = null) {
    if (originalName == null) {
      originalName = "";
    }
    if (this.state.selectedMode != null) {
    if (Object.keys(this.state.selectedMode).includes('siteName')) {
      if (this.state.siteData.connection == "FM") {
        for (let i = 0; i < this.state.siteData.sites.length; i++) {
          if (originalName == this.state.siteData.sites[i].siteName) {
            continue;
          } else {
            if (offeredName == this.state.siteData.sites[i].siteName) {
              return false;
            }
          }
        }
        return true;
      }
      return true;
    } return true;}
    let index = -1;
    for (let i = 0; i < Object.keys(this.state.modeData).length; i++) {
      if (originalName.toLowerCase() == offeredName) {
        continue;
      }
      if (Object.keys(this.state.modeData)[i] == offeredName) {
        index = i
      }
    }
    return index == -1;
  }

  isNewIdValid(offeredName, originalName = null) {
    if (this.state.selectedMode == null) return true;
    if (Object.keys(this.state.selectedMode).includes('siteId')) {
      if (this.state.siteData.connection == "FM") {
        for (let i = 0; i < this.state.siteData.sites.length; i++) {
          if (originalName == this.state.siteData.sites[i].siteId) {
            continue;
          } else {
            if (offeredName == this.state.siteData.sites[i].siteId) {
              return false;
            }
          }
        }
        return true;
      }
      return true;
    }
    let index = -1;
    for (let i = 0; i < Object.keys(this.state.modeData).length; i++) {
      if (originalName == offeredName) {
        continue;
      }
      if (Object.keys(this.state.modeData)[i] == offeredName) {
        index = i
      }
    }
    return index == -1;
  }


  layerWhenSelectingDisabled() {
    if (this.state.selectedMode != null) {
      return 'rgba(220, 220, 220, .5)';
    } else {
      return '';
    }
  }

  layerWhenEditingDisabled() {
    if (this.editingIsDisabled()) {
      return 'rgba(220, 220, 220, .5)';
    } else {
      return '';
    }
  }

  handleLock = (event) => {
    this.setState({
      locked: !this.state.locked
    })
  }

  handleDropDown = (event) => {
    let newSiteData = {
      'connection' : event.target.value,
      'sites' : []
    };
    if (event.target.value == "SC") {
      newSiteData = {
        'connection' : event.target.value,
        'siteName' : '',
        siteControllerPort: ''
      };
    }
    this.setState({
      connection: event.target.value,
      siteData: newSiteData,
      locked: true
        // input: newInput
    });
}
  updateUpdatingSite(newSiteData) {
    this.setState({
      selectedSiteWithUpdates: newSiteData
    });
  }

  render() {
    return <>
      {
      <>
      <div style={{ width: '95%', margin: 'auto', paddingBottom: '10px', paddingTop: '10px'}}>
            <FormControl 
              disabled={false}
              style={{ minWidth: '240px'}} 
              variant="outlined">
              <InputLabel>Fleet/Site Configuration</InputLabel>
                  <Select
                  label="Fleet/Site Configuration"
                  name="Fleet/Site Configuration"
                  disabled={!this.editingIsDisabled() || this.state.locked}

                  onChange={this.handleDropDown}
                  value={this.state.connection}
                  >
                    <MenuItem value="FM">Fleet Manager</MenuItem>
                    <MenuItem value="SC">Site Controller</MenuItem>
                  </Select>
                  {!this.state.locked &&
                    <FormHelperText>Changing is potentially destructive</FormHelperText>
                  }
            </FormControl>
            <IconButton
              disabled={this.state.selectedMode != null}
              onClick={this.handleLock}
              size="large">{this.state.locked ? <LockIcon></LockIcon> : <LockOpenIcon></LockOpenIcon>}</IconButton>
            {/* <IconButton onClick={() => this.onDeletePressed()} disabled={this.isDisabled() || !this.state.showConfirmation} style={{color:`${this.state.showConfirmation ? '#4BC940' : '#d1d1d1'}` }}><CheckIcon/></IconButton>
            <IconButton onClick={() => this.cancelDelete()} disabled={this.isDisabled() || !this.state.showConfirmation} style={{color:`${this.state.showConfirmation ? '#E33B3B' : '#d1d1d1'}` }}><CloseIcon/></IconButton> */}
          </div>
        <Paper style={{ width: '95%', height: '72vh', margin: 'auto', overflow: 'hidden' }} variant="outlined">
          <div style={{
            borderColor: 'transparent',
            background: `${this.layerWhenSelectingDisabled()}`,
            float: 'left',
            width: '30%',
            height: '90%',
            position: 'relative',
            display: 'flex',
            flexDirection: 'column',
            alignItems: 'center'
          }}>
            <ModeList
              modeData={this.state.modeData}
              siteData={this.state.siteData}
              connection={this.state.connection}
              selectedMode={this.state.selectedMode}
              selectMode={this.selectMode}
              submitNewMode={this.submitNewMode}
              submitNewSite={this.submitNewSite}
              isNewNameValid={this.isNewNameValid}
              isNewIdValid={this.isNewIdValid}
            />
          </div>
          <div style={{
            background: `${this.layerWhenEditingDisabled()}`,
            borderColor: 'transparent',
            borderColor: 'transparent',
            float: 'left',
            width: '70%',
            height: '90%',
            position: 'relative',
            display: 'flex',
            flexDirection: 'column',
            alignItems: 'center'
          }}>
            <div style={{ position: 'relative', height: '8%', width: '100%' }}>
              <AppBar position="static">
                <Tabs
                  TabIndicatorProps={{ style: { background: '#FFF' } }}
                  value={this.state.tabIndex}
                  onChange={this.handleTabsChange}
                  centered
                  indicatorColor="primary"
                  textColor="inherit"
                >
                  <Tab disabled={this.state.selectedMode == null} label="Info" />
                  {this.state.selectedMode != null && !Object.keys(this.state.selectedMode).includes('siteName') && <Tab disabled={this.state.selectedMode == null} label="Variables" />}
                  {this.state.selectedMode != null && !Object.keys(this.state.selectedMode).includes('siteName') && <Tab disabled={this.state.selectedMode == null} label="Constants" />}
                </Tabs>
              </AppBar>
            </div>
            <div style={{ position: 'relative', height: '92%', width: '100%' }}>
              <TabPanel value={this.state.tabIndex} index={0}>
                {this.state.selectedMode != null &&
                  <InfoList isNewNameValid={this.isNewNameValid}
                  isNewIdValid={this.isNewIdValid}
                  connection={this.state.connection}
                    confirmDeleteButtonPressed={this.confirmDeleteButtonPressed}
                    isDisabled={this.editingIsDisabled()}
                    siteData={this.state.siteData}
                    handleInfoListError={this.handleInfoListError}
                    selectedMode={this.state.selectedMode}
                    selectedModeWithDeletions={this.state.selectedModeWithDeletions}
                    updatingMode={this.state.updatingMode}
                    updateUpdatingModeWith={this.updateUpdatingModeWith}
                    updateUpdatingSite={this.updateUpdatingSite}
                    selectedSiteWithUpdates={this.state.selectedSiteWithUpdates} />
                }
              </TabPanel>
              <TabPanel value={this.state.tabIndex} index={1}>
                <VariableList
                  handleVariableListError={this.handleVariableListError}
                  updatingMode={this.state.updatingMode}
                  selectedMode={this.state.selectedModeWithDeletions}
                  updateUpdatingModeWith={this.updateUpdatingModeWith}
                  updateUpdatingModeWhenVarDeleted={this.updateUpdatingModeWhenVarDeleted}
                  handleConfirmClick={this.handleConfirmClick}
                  isConstant={false} />
              </TabPanel>
              <TabPanel
                value={this.state.tabIndex}
                index={2}>
                <VariableList
                  handleVariableListError={this.handleVariableListError}
                  updateUpdatingModeWith={this.updateUpdatingModeWith}
                  updateUpdatingModeWhenVarDeleted={this.updateUpdatingModeWhenVarDeleted}
                  selectedMode={this.state.selectedModeWithDeletions}
                  updatingMode={this.state.updatingMode}
                  isConstant={true} />
              </TabPanel>
            </div>
          </div>
          <div style={{ position: 'relative', width: '100%', height: '10%', display: 'flex', justifyContent: 'center', }}>
            <div style={{ right: '0', right: '4px', position: 'absolute', top: '50%', transform: 'translateY(-50%)' }}>
              <Button onClick={() => this.buttonPressed('cancel')}
                disabled={this.editingIsDisabled()}
                variant="outlined"
                color="secondary"
                style={{ marginRight: '10px' }}>CANCEL
              </Button>
              <Button onClick={() => this.buttonPressed('save')}
                disabled={this.editingIsDisabled() || this.state.errors > 0}
                variant="contained"
                color="primary">Save Changes
              </Button>
              <IconButton
                disabled={!this.state.showConfirmation}
                onClick={() => this.confirmButton()}
                style={{ color: `${this.state.showConfirmation ? '#4BC940' : LIGHT_GRAY}` }}
                size="large"><CheckIcon /></IconButton>
              <IconButton
                disabled={!this.state.showConfirmation}
                onClick={() => this.cancelButton()}
                style={{ color: `${this.state.showConfirmation ? '#E33B3B' : LIGHT_GRAY}` }}
                size="large"><CloseIcon /></IconButton>
            </div>
          </div>
        </Paper>
        </>}
    </>;
  }
}
export default withStyles(LoadingHOC(SchedulerConfig), STYLES_SCHEDULER_CONFIGURATION);
