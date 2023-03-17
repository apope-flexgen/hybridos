import React from 'react';
import TextField from '@mui/material/TextField'
import { withStyles } from 'tss-react/mui';
import { STYLES_SCHEDULER_CONFIGURATION } from '../../styles';

import Button from '@mui/material/Button';
import IconButton from '@mui/material/IconButton';

import CheckIcon from '@mui/icons-material/Check';
import CloseIcon from '@mui/icons-material/Close';
import DeleteIcon from '@mui/icons-material/Delete';
import UndoIcon from '@mui/icons-material/Undo';
import InfoIcon from '@mui/icons-material/Info';
import PaletteIcon from '@mui/icons-material/Palette';

import List from '@mui/material/List';
import ListItem from '@mui/material/ListItem';
import ListItemIcon from '@mui/material/ListItemIcon';

const LIGHT_BLUE = 'rgba(63,81,181,.1)';
const LIGHT_GRAY = '#D1D1D1';

class InfoList extends React.Component {
    constructor(props) {
        super(props);
        let changedIP = "";
        let changedPort = "";
        let key = props.selectedMode != null ? Object.keys(props.updatingMode)[0] : '';
        if (Object.keys(props.selectedMode).includes('siteName')) {
            key = props.selectedMode.siteName;
            if (Object.keys(props.selectedMode).includes('siteControllerIP')) {
                changedIP = props.selectedMode.siteControllerIP
            }
            if (Object.keys(props.siteData.connection == "SC")) {
                changedPort = Object.keys(props.selectedMode).includes("siteControllerPort") ? props.selectedMode.siteControllerPort : "";
            }
        }
        this.state = {
            updatingMode: props.updatingMode,
            selectedMode: props.selectedMode,
            changedName: key,
            changedId: props.selectedMode.siteId,
            changedIP: changedIP,
            changedPort: changedPort,
            changedColor: props.selectedMode != null && !Object.keys(props.selectedMode).includes('siteName') ? props.updatingMode[key].color_code : LIGHT_GRAY,
            showConfirmation: false,
            nameError: false,
            idError: false
        }

        this.handleColorChange = this.handleColorChange.bind(this);
        this.handleNameChange = this.handleNameChange.bind(this);
        this.handleIdChange = this.handleIdChange.bind(this);
        this.handleIPChange = this.handleIPChange.bind(this);
        this.handlePortChange = this.handlePortChange.bind(this);
    }

    /**
     * Returns proper coloring - blue if field has changed, transparent if not.
     */
    getBackgroundColor(modeData) {
        let key = Object.keys(modeData)[0];
        let colorCode = modeData[key].color_code;
        if (colorCode == '') {
            return '';
        }
        if (colorCode != null || colorCode == '') {
            return colorCode;
        } else {
            return LIGHT_GRAY;
        }
    }

    reset() {
        this.setState({
            changedName: '',
            changedColor: '',
            showConfirmation: false,
            nameError: false
        });
    }

    changeIdInJSON(newId) {
        let newSiteData = JSON.parse(JSON.stringify(this.props.selectedSiteWithUpdates));
            newSiteData.siteId = newId;
            this.props.updateUpdatingSite(newSiteData)
            
            return;
    }
    
    changeNameInJSON(newName) {
        if (Object.keys(this.state.selectedMode).includes('siteName')) {
            let newSiteData = JSON.parse(JSON.stringify(this.props.selectedSiteWithUpdates));
            newSiteData.siteName = newName;
            this.props.updateUpdatingSite(newSiteData)
            
            return;
        }
        let modeTitle = Object.keys(this.state.updatingMode)[0];
        let newModeData = this.state.updatingMode[modeTitle];
        
        let theNewJSON = {
            [newName] : newModeData
        };

        modeTitle = Object.keys(this.props.selectedModeWithDeletions)[0];
        newModeData = this.props.selectedModeWithDeletions[modeTitle];
        
        let newSelectedModeWithDeletions = {
            [newName] : newModeData
        };
        this.props.updateUpdatingModeWith(theNewJSON, newSelectedModeWithDeletions);
    }

    changeColorInJSON(newColor, updateState=false) {
        newColor = newColor.toUpperCase();
        
        if (newColor == null || newColor == '') {
            newColor = LIGHT_GRAY;
        }
        let modeTitle = Object.keys(this.state.updatingMode)[0];
        this.state.updatingMode[modeTitle].color_code = newColor;
        if (updateState) {
            this.props.updateUpdatingModeWith(JSON.parse(JSON.stringify(this.state.updatingMode)));
        }
        
    }

    handleNameChange = (event) => {
        let isNameError = !this.props.isNewNameValid(event.target.value, !Object.keys(this.state.selectedMode).includes('siteName') ? Object.keys(this.state.selectedMode)[0] : this.state.selectedMode.siteName);
        if(this.state.nameError && !isNameError) this.props.handleInfoListError(-1);
        if(!this.state.nameError && isNameError) this.props.handleInfoListError(1);
        this.setState({
            changedName: event.target.value,
            nameError: !this.props.isNewNameValid(event.target.value, !Object.keys(this.state.selectedMode).includes('siteName') ? Object.keys(this.state.selectedMode)[0] : this.state.selectedMode.siteName) || event.target.value == ""
        }, () => this.changeNameInJSON(this.state.changedName));
    }

    handleIdChange = (event) => {
        let newValue = event.target.value;
        // [^a-zA-Z0-9_]*$
        newValue = newValue.replaceAll(/[^a-zA-Z0-9_]/g, '');


        let isNameError = !this.props.isNewIdValid(newValue, !Object.keys(this.state.selectedMode).includes('siteId') ? Object.keys(this.state.selectedMode)[0] : this.state.selectedMode.siteId);
        if(this.state.idError && !isNameError) this.props.handleInfoListError(-1);
        if(!this.state.idError && isNameError) this.props.handleInfoListError(1);
        this.setState({
            changedId: newValue,
            idError: !this.props.isNewIdValid(newValue, !Object.keys(this.state.selectedMode).includes('siteId') ? Object.keys(this.state.selectedMode)[0] : this.state.selectedMode.siteId) || newValue == ""
        }, () => this.changeIdInJSON(this.state.changedId));
    }

    handleIPChange = (event) => {
        this.setState({
            changedIP: event.target.value
        }, () => {
            let newSiteData = JSON.parse(JSON.stringify(this.props.selectedSiteWithUpdates))
            newSiteData.siteControllerIP = this.state.changedIP;
            this.props.updateUpdatingSite(newSiteData)
        });
    }

    handlePortChange = (event) => {
        this.setState({
            changedPort: event.target.value
        }, () => {
            let newSiteData = JSON.parse(JSON.stringify(this.props.selectedSiteWithUpdates))
            newSiteData.siteControllerPort = this.state.changedPort;
            this.props.updateUpdatingSite(newSiteData);
        });
    }

    handleColorChange = (event) => {
        this.setState({
            changedColor: event.target.value,
        }, () => this.changeColorInJSON(this.state.changedColor));
    }

    deleteMode() {
        this.setState({
            showConfirmation: true
        });
    }

    confirmDeleteButtonPressed() {
        this.props.confirmDeleteButtonPressed()
        this.reset();
    }

    cancelDeleteButtonPressed() {
        this.setState({
            showConfirmation: false
        });
    }

    getShadedColor(isNameField, isIPAsOpposedToPort = 0) {
        if (isIPAsOpposedToPort == 1) {
            if (this.state.selectedMode.siteControllerIP != this.state.changedIP) {
                return LIGHT_BLUE
            }
            return '';
        } else if (isIPAsOpposedToPort == 2) {
            if (this.state.selectedMode.siteControllerPort != this.state.changedPort) {
                if (!Object.keys(this.state.selectedMode).includes('siteControllerIP') && this.state.changedPort == "") {
                    return '';
                }

                return LIGHT_BLUE
            }
            return '';
        }  else if (isIPAsOpposedToPort == 3) {
            if (this.state.selectedMode.siteId != this.state.changedId) {


                return LIGHT_BLUE
            }
            return '';
        }

        if (isNameField && this.state.selectedMode != null && Object.keys(this.state.selectedMode).includes('siteName')) {
            if (this.state.selectedMode.siteName != this.state.changedName) {
                return LIGHT_BLUE
            }
            return '';
        }
        if (isNameField && this.state.selectedMode != null && Object.keys(this.state.selectedMode)[0] != this.state.changedName) {
            return LIGHT_BLUE;
        }
        if (!isNameField && this.state.selectedMode != null && this.getBackgroundColor(this.state.selectedMode).toLowerCase() != this.state.changedColor.toLowerCase()) {
            return LIGHT_BLUE;
        }
        return '';
    }


    undo(isNameField, isIPFieldAsOpposedToPort = 0) {
        if (isIPFieldAsOpposedToPort == 1) {
            this.setState({
                changedIP: this.props.selectedMode.siteControllerIP
            }, () => {
                let newSiteData = JSON.parse(JSON.stringify(this.props.selectedSiteWithUpdates));
                newSiteData.siteControllerIP = this.props.selectedMode.siteControllerIP;
                this.props.updateUpdatingSite(newSiteData)
            })
            return;
        } else if (isIPFieldAsOpposedToPort == 2) {
            this.setState({
                changedPort: Object.keys(this.props.selectedMode).includes('siteControllerPort') ? this.props.selectedMode.siteControllerPort : ""
            }, () => {
                let newSiteData = JSON.parse(JSON.stringify(this.props.selectedSiteWithUpdates));
                newSiteData.siteControllerPort = this.props.selectedMode.siteControllerPort;
                this.props.updateUpdatingSite(newSiteData)
            });
            return;
        } else if (isIPFieldAsOpposedToPort == 3) { //changedId
            this.setState({
                changedId: Object.keys(this.props.selectedMode).includes('siteId') ? this.props.selectedMode.siteId : "",
                idError: false
            }, () => {
                
                this.changeIdInJSON(this.state.changedId)
            });
            return;
        }
        if (isNameField) {
            this.props.handleInfoListError(-1);
            if (Object.keys(this.state.selectedMode).includes('siteName')) {
                this.changeNameInJSON(this.props.selectedMode.siteName);
            } else {
                this.changeNameInJSON(Object.keys(this.state.selectedMode)[0]);
            }
            this.setState({
                changedName: Object.keys(this.state.selectedMode).includes('siteName') ? this.state.selectedMode.siteName : Object.keys(this.state.selectedMode)[0],
                nameError: false
            });
        } else {
            this.changeColorInJSON(this.getBackgroundColor(this.state.selectedMode), true);
            this.setState({
                changedColor: this.getBackgroundColor(this.state.selectedMode),
                nameError: false
            });
        }
    }

    isDisabled() {
        if (this.state.selectedMode != null && Object.keys(this.state.selectedMode)[0] == 'default') {
            return true;
        }
        return this.props.isDisabled;
    }
    
    render() {
        let portUndoEnabled = this.state.selectedMode != null && (Object.keys(this.state.selectedMode).includes('siteName') && (this.state.selectedMode.siteControllerPort != this.state.changedPort));
        if (this.state.selectedMode != null && (Object.keys(this.state.selectedMode).includes('siteName') && (this.state.connection == "SC"  ))) {
        }
        if (this.state.selectedMode != null && (Object.keys(this.state.selectedMode).includes('siteName') && (this.state.selectedMode.connection == "SC" && (!Object.keys(this.state.selectedMode).includes('siteControllerPort') && this.state.changedPort != "")))) {
            portUndoEnabled = true;
        } else if (this.state.selectedMode != null && (Object.keys(this.state.selectedMode).includes('siteName') && (this.state.selectedMode.connection == "SC" && (!Object.keys(this.state.selectedMode).includes('siteControllerPort') && this.state.changedPort == "")))) {
            portUndoEnabled = false
        }
        let colorForPortUndo = portUndoEnabled ? '#3F51B5' : "#d1d1d1";
        if (this.state.selectedMode != null && this.state.selectedMode.siteControllerPort != this.state.changedPort) {
            colorForPortUndo = '#3F51B5';
            if (!Object.keys(this.state.selectedMode).includes('siteControllerIP') && this.state.changedPort == "") {
                colorForPortUndo = '#d1d1d1';
            }
        }


        let nameErrorText = !this.props.isNewNameValid(this.state.changedName, !Object.keys(this.state.selectedMode).includes('siteName') ? Object.keys(this.state.selectedMode)[0] : this.state.selectedMode.siteName) ? 'Name already exists' : 'Name must not be empty'
        
        let idErrorText = !this.props.isNewIdValid(this.state.changedId, !Object.keys(this.state.selectedMode).includes('siteId') ? Object.keys(this.state.selectedMode)[0] : this.state.selectedMode.siteId) ? 'ID already exists' : 'ID must not be empty'
        return <>
            <div align="left" style={{  width: '95%', paddingLeft: '10px', paddingTop: '10px', }}>
                

            <List>
                {/* {!Object.keys(this.props.selectedMode).includes("siteName") && */}
                <ListItem style={{ backgroundColor: `${this.getShadedColor(true)}` }}>
                    <ListItemIcon>
                        <InfoIcon />
                    </ListItemIcon>
                    <TextField disabled={this.isDisabled()}
                        style={{  width: '215px'}}
                        label='Name'
                        variant="outlined"
                        size='small'
                        error={this.state.nameError || !this.props.isNewNameValid(this.state.changedName, !Object.keys(this.state.selectedMode).includes('siteName') ? Object.keys(this.state.selectedMode)[0] : this.state.selectedMode.siteName)}
                        helperText={!this.props.isNewNameValid(this.state.changedName, !Object.keys(this.state.selectedMode).includes('siteName') ? Object.keys(this.state.selectedMode)[0] : this.state.selectedMode.siteName) || this.state.nameError ? nameErrorText : ""}
                        onChange={this.handleNameChange}
                        value={this.state.changedName}
                        />
                    <IconButton
                        disabled={!(this.state.selectedMode != null && ((Object.keys(this.state.selectedMode).includes('siteName') && this.state.selectedMode.siteName != this.state.changedName) || (!Object.keys(this.state.selectedMode).includes('siteName') && Object.keys(this.state.selectedMode)[0] != this.state.changedName)))}
                        style={{color: `${ (this.state.selectedMode != null && ((Object.keys(this.state.selectedMode).includes('siteName') && this.state.selectedMode.siteName != this.state.changedName) || (!Object.keys(this.state.selectedMode).includes('siteName') && Object.keys(this.state.selectedMode)[0] != this.state.changedName)))? '#3F51B5' : '#d1d1d1'}`}}
                        onClick={() => this.undo(true)}
                        size="large"><UndoIcon/></IconButton>
                </ListItem>

                {Object.keys(this.props.selectedMode).includes('siteName') && Object.keys(this.props.selectedMode).includes('siteId') &&

                <ListItem style={{ backgroundColor: `${this.getShadedColor(true, 3)}` }}>
                    <ListItemIcon>
                        <InfoIcon />
                    </ListItemIcon>
                    <TextField disabled={this.isDisabled()}
                        style={{  width: '215px'}}
                        label='ID'
                        variant="outlined"
                        size='small'
                        error={this.state.idError || !this.props.isNewIdValid(this.state.changedId, !Object.keys(this.state.selectedMode).includes('siteId') ? Object.keys(this.state.selectedMode)[0] : this.state.selectedMode.siteId)}
                        helperText={!this.props.isNewIdValid(this.state.changedId, !Object.keys(this.state.selectedMode).includes('siteId') ? Object.keys(this.state.selectedMode)[0] : this.state.selectedMode.siteId) || this.state.idError ? idErrorText : ""}
                        onChange={this.handleIdChange}
                        value={this.state.changedId}
                        />
                    <IconButton
                        disabled={!(this.state.selectedMode != null && ((Object.keys(this.state.selectedMode).includes('siteId') && this.state.selectedMode.siteId != this.state.changedId) || (!Object.keys(this.state.selectedMode).includes('siteId') && Object.keys(this.state.selectedMode)[0] != this.state.siteId)))}
                        style={{color: `${ (this.state.selectedMode != null && ((Object.keys(this.state.selectedMode).includes('siteId') && this.state.selectedMode.siteId != this.state.changedId) || (!Object.keys(this.state.selectedMode).includes('siteId') && Object.keys(this.state.selectedMode)[0] != this.state.changedId)))? '#3F51B5' : '#d1d1d1'}`}}
                        onClick={() => this.undo(true, 3)}
                        size="large"><UndoIcon/></IconButton>
                </ListItem>}

                {Object.keys(this.props.selectedMode).includes('siteName') && Object.keys(this.props.selectedMode).includes('siteControllerIP') &&
                <ListItem style={{ backgroundColor: `${this.getShadedColor(true, 1)}` }}>
                    <ListItemIcon>
                        <InfoIcon />
                    </ListItemIcon>
                    <TextField disabled={this.isDisabled()}
                        style={{  width: '215px'}}
                        label='IP Address'
                        variant="outlined"
                        size='small'
                        error={!(/^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/.test(this.state.changedIP))}
                        helperText={(/^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/.test(this.state.changedIP)) ? '' : 'Invalid IP Address'}
                        onChange={this.handleIPChange}
                        value={this.state.changedIP}
                        />
                    <IconButton
                        disabled={!(this.state.selectedMode != null && (Object.keys(this.state.selectedMode).includes('siteName') && this.state.selectedMode.siteControllerIP != this.state.changedIP)) }
                        style={{color: `${ (this.state.selectedMode != null && ((Object.keys(this.state.selectedMode).includes('siteName') && this.state.selectedMode.siteControllerIP != this.state.changedIP) ))? '#3F51B5' : '#d1d1d1'}`}}
                        onClick={() => this.undo(true, 1)}
                        size="large"><UndoIcon/></IconButton>
                </ListItem>

                }

                {Object.keys(this.props.selectedMode).includes('siteName') && Object.keys(this.props.siteData.connection == "SC") &&
                <ListItem style={{ backgroundColor: `${this.getShadedColor(true, 2)}` }}>
                    <ListItemIcon>
                        <InfoIcon />
                    </ListItemIcon>
                    <TextField disabled={this.isDisabled()}
                        style={{  width: '215px'}}
                        label='Port'
                        variant="outlined"
                        size='small'
                        type='number'
                        error={false}
                        helperText= {this.props.connection == "SC" ? "Include a port number for Fleet-connected Site Controller" : ""}
                        onChange={this.handlePortChange}
                        value={this.state.changedPort}
                        />
                    <IconButton
                        disabled={!portUndoEnabled }
                        style={{color: `${colorForPortUndo}`}}
                        onClick={() => this.undo(true, 2)}
                        size="large"><UndoIcon/></IconButton>
                </ListItem>

                }

                {!Object.keys(this.props.selectedMode).includes("siteName") &&
                <ListItem style={{ backgroundColor: `${this.getShadedColor(false)}` }}>
                    <ListItemIcon>
                        <PaletteIcon/>
                    </ListItemIcon>
                    
                    <input disabled={this.isDisabled()} style={{ height:'50px', width:"215px" }} type="color" value={this.props.isDisabled || this.state.changedColor == '' ? LIGHT_GRAY : this.state.changedColor} onChange={this.handleColorChange}/>
                    <IconButton
                        disabled={!(this.state.selectedMode != null && this.getBackgroundColor(this.state.selectedMode) != this.state.changedColor)}
                        onClick={() => this.undo(false)}
                        style={{color: `${(this.state.selectedMode != null && this.getBackgroundColor(this.state.selectedMode) != this.state.changedColor) ? '#3F51B5' : '#d1d1d1'}`}}
                        size="large"><UndoIcon/></IconButton>

                </ListItem>}
                {(!Object.keys(this.props.selectedMode).includes("siteName") || (Object.keys(this.props.selectedMode).includes("siteName") && this.props.siteData["connection"] == 'FM')) &&
                <ListItem>
                    <ListItemIcon>
                        <DeleteIcon/>
                    </ListItemIcon>
                    <Button disabled={this.state.showConfirmation || this.isDisabled()} onClick={() => this.deleteMode()}  variant="outlined" 
                                color="secondary">Delete {Object.keys(this.props.selectedMode).includes('siteName') ? 'Site' : 'Mode'}</Button>
                    <IconButton
                        disabled={!this.state.showConfirmation}
                        onClick={() => this.confirmDeleteButtonPressed()}
                        style={{color:`${this.state.showConfirmation && !this.isDisabled() ? '#4BC940' : LIGHT_GRAY}`}}
                        size="large"><CheckIcon/></IconButton>
                    <IconButton
                        disabled={!this.state.showConfirmation}
                        onClick={() => this.cancelDeleteButtonPressed()}
                        style={{color:`${this.state.showConfirmation && !this.isDisabled() ? '#E33B3B' : LIGHT_GRAY}`}}
                        size="large"><CloseIcon/></IconButton>
                </ListItem>}
            </List>
            </div>
        </>;
    }
}

export default withStyles(InfoList, STYLES_SCHEDULER_CONFIGURATION);