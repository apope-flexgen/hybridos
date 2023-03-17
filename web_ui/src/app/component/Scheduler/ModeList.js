import React from 'react';
import Button from '@mui/material/Button';
import TextField from '@mui/material/TextField';
import Typography from '@mui/material/Typography';
import { withStyles } from 'tss-react/mui';
import { STYLES_SCHEDULER_CONFIGURATION } from '../../styles';

class ModeList extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            creatingNewMode: false,
            creatingNewSite: false,
            isValidNewMode: false,
            newModeName: '',
            newSiteName: '',
            newSiteIP: '',
            newSiteId: '',
            newSitePort: '9000',
            error: false,
        };

        this.createMode = this.createMode.bind(this);
    }

    getBackgroundColor(modeData) {
        let colorCode = modeData.color_code;
        if (colorCode != null && colorCode != '') {
            return colorCode;
        } else {
            return '#d1d1d1';
        }
    }

    getInCreationButton() {
        return 
    }

    handleModeNameChange = (event) => {
        let error = !this.props.isNewNameValid(event.target.value.toLowerCase());
        if (event.target.value.length > 0) {
            this.setState({
                newModeName: event.target.value,
                isValidNewMode: !error,
                error: error
            });
        } else {
            this.setState({
                newModeName: event.target.value,
                isValidNewMode: false,
                error: error
            });
        }
    }

    handleSiteIPChange = (event) => {
        // let error = !this.props.isNewNameValid(event.target.value.toLowerCase());
        // if (event.target.value.length > 0) {
        //     this.setState({
        //         newModeName: event.target.value.toLowerCase(),
        //         isValidNewMode: !error,
        //         error: error
        //     });
        // } else {
            this.setState({
                newSiteIP: event.target.value.toLowerCase(),
                // isValidNewMode: false,
                // error: error
            });
        // }
    }

    handleSitePortChange = (event) => {
        // let error = !this.props.isNewNameValid(event.target.value.toLowerCase());
        // if (event.target.value.length > 0) {
        //     this.setState({
        //         newModeName: event.target.value.toLowerCase(),
        //         isValidNewMode: !error,
        //         error: error
        //     });
        // } else {
            this.setState({
                newSitePort: event.target.value.toLowerCase(),
                // isValidNewMode: false,
                // error: error
            });
        // }
    }

    handleSiteNameChange = (event) => {
        // let error = !this.props.isNewNameValid(event.target.value.toLowerCase());
        // if (event.target.value.length > 0) {
        //     this.setState({
        //         newModeName: event.target.value.toLowerCase(),
        //         isValidNewMode: !error,
        //         error: error
        //     });
        // } else {
            this.setState({
                newSiteName: event.target.value,
                // isValidNewMode: false,
                // error: error
            });
        // }
    }

    handleSiteIdChange = (event) => {
        let newValue = event.target.value;
        newValue = newValue.replaceAll(/[^a-zA-Z0-9_]/g, '');
        // let error = !this.props.isNewNameValid(event.target.value.toLowerCase());
        // if (event.target.value.length > 0) {
        //     this.setState({
        //         newModeName: event.target.value.toLowerCase(),
        //         isValidNewMode: !error,
        //         error: error
        //     });
        // } else {
            this.setState({
                newSiteId: newValue,
                // isValidNewMode: false,
                // error: error
            });
        // }
    }
    
    isSelected(modeDataTitle, isModeAsOpposedToSite) {
        let selectedMode = this.props.selectedMode;
        

        if (selectedMode != null) {
            if (Object.keys(selectedMode).includes('siteName') && modeDataTitle == selectedMode.siteName && !isModeAsOpposedToSite) {
                return true;
            } else if (isModeAsOpposedToSite) {
                return modeDataTitle == Object.keys(this.props.selectedMode)[0];
            }
            return false;
        } else {
            return false;
        }
    }

    getModeButtons() {
        const modeData = this.props.modeData;
        let keys = Object.keys(modeData);
        let buttons = [];
        for (let i = 0; i < keys.length; i++) {
            let title = keys[i];
            let includeOutline = this.isSelected(title, true);
            let selMode ={};
            selMode[title] = modeData[title];
            buttons.push(
                <Button variant = {includeOutline ? "outlined" : "contained"} //false replaced with includeOutline var
                disabled={this.state.creatingNewMode || this.state.creatingNewSite || (!this.isSelected(title, true) && this.props.selectedMode != null)}
                onClick={() => this.props.selectMode(selMode)}
                style={{ border: `${includeOutline ? "2px solid": "0px solid"}`, margin: '4px', minWidth: '95%', backgroundColor: `${this.getBackgroundColor(modeData[title])}`}}>
                    <div style = {{ width: '100%', display: 'flex', flexDirection: 'column', alignItems: 'center' }}>
                        <Typography>
                            {title}
                        </Typography>

                    </div>
                </Button>
            );
        }
        
        return buttons;
    }

    getSiteButtons() {
        const siteData = this.props.siteData;
        if (siteData == null) return [];
        if (Object.keys(siteData).length == 0) return [];
        let sitesInfo = [];
        if (siteData['connection'] == 'FM') {
            sitesInfo = siteData['sites'];
        } else {
            sitesInfo.push({
                "siteName" : siteData["siteName"]
            })
        }
        sitesInfo.sort((a, b) => (a.siteName > b.siteName) ? 1 : -1)
        // let sitesInfo = [];
        let buttons = [];
        for (let i = 0; i < sitesInfo.length; i++) {
            let siteName = sitesInfo[i]["siteName"];
            if (siteName == "") continue;
            let includeOutline = this.isSelected(siteName, false);
            let siteData = sitesInfo[i]
            // selMode[title] = siteData[title];
            buttons.push(
                <Button variant = {includeOutline ? "outlined" : "contained"} //false replaced with includeOutline var
                disabled={this.state.creatingNewMode || this.state.creatingNewSite || (!this.isSelected(siteName, false) && this.props.selectedMode != null)}
                onClick={() => this.props.selectMode(siteData, false)}
                style={{ border: `${includeOutline ? "2px solid": "0px solid"}`, margin: '4px', minWidth: '95%', backgroundColor: "#d1d1d1"}}>
                    <div style = {{ width: '100%', display: 'flex', flexDirection: 'column', alignItems: 'center' }}>
                        <Typography>
                            {siteName}
                        </Typography>

                    </div>
                </Button>
            );
        }
        
        return buttons;
    }

    getHelperText() {
        if (this.state.error) {
            return 'Name already exists';
        } else {
            return '';
        }
    }

    getAddModeTextField() {
        return (
            <div style = {{ height: '54px', width: '50%', display: 'flex', flexDirection: 'column', alignItems: 'center' }}>
                        <TextField
                        style={{  width: '100%', height: '10px', marginBottom: '10px'}}
                        label='Name'
                        variant="outlined"
                        size='small'
                        error={this.state.error}
                        helperText={this.getHelperText()}
                        onChange={this.handleModeNameChange}
                        value={this.state.newModeName}
                        />

                    </div>
        )
    }

    getAddSiteTextFields() {
        let height = '194px'
        // if (this.props.connection == 'FM') {
        //     height = '194px'
        // }
        return (
            <div style = {{ height: `${height}`, width: '50%', display: 'flex', flexDirection: 'column', alignItems: 'center', alignContent: 'space-between' }}>
                        <TextField
                        style={{ marginBottom: '10px'}}
                        label='Site Name'
                        variant="outlined"
                        size='small'
                        error={this.errorForSiteNameEmpty() || this.errorForSiteNameDuplicate()}
                        // helperText={'hello'}
                        
                        onChange={this.handleSiteNameChange}
                        value={this.state.newSiteName}
                        />

                        <TextField
                        style={{ marginBottom: '10px'}}
                        label='Site ID'
                        variant="outlined"
                        size='small'
                        error={this.errorForSiteIdEmpty()}
                        // helperText={'hello'}
                        
                        onChange={this.handleSiteIdChange}
                        value={this.state.newSiteId}
                        />

                        {this.props.connection != "SC" &&
                        
                            <TextField
                            style={{ marginBottom: '10px'}}
                            label='IP Address'
                            variant="outlined"
                            size='small'
                            error={this.errorForIP()}
                            // helperText={this.getHelperText()}
                            onChange={this.handleSiteIPChange}
                            value={this.state.newSiteIP}
                            />}
                        {/* {this.props.connection != "DSC" && */}
                            <TextField
                            style={{ marginBottom: '10px'}}
                            label='Port'
                            variant="outlined"
                            size='small'
                            type='number'
                            error={this.errorForPort()}
                            helperText= {this.props.connection == "SC" ? "Include a port number for Fleet-connected Site Controller" : ""}
                            onChange={this.handleSitePortChange}
                            value={this.state.newSitePort}
                            />
                        
                        {/* } */}

                    </div>
        )
    }

    getModeAddButton() {
        return(
            <Button disabled={this.state.creatingNewMode || this.props.selectedMode != null} variant="contained" color="primary" onClick={() => this.createMode()}>
                New Mode
            </Button>
        )
    }

    getSiteAddButton() {
        return(
            <Button style={{marginRight: '10px'}}disabled={this.state.creatingNewMode || this.props.selectedMode != null || this.props.connection == "" || (this.props.connection != "FM" && this.getSiteButtons().length > 0)} variant="contained" color="primary" onClick={() => this.createSite()}>
                New Site
            </Button>
        )
    }

    createMode() {
        this.props.selectMode(null);
        this.setState({creatingNewMode: true})
    }

    createSite() {
        this.props.selectMode(null);
        this.setState({creatingNewSite: true, isValidNewMode: true})
    }

    getCancelButton() {
        return(
            <Button style={{ marginRight: '10px' }} disabled={false} variant="outlined" color="secondary" onClick={() => this.cancelCreateMode()}>
                Cancel
            </Button>
        )
    }

    reset() {
        this.setState({
            creatingNewMode: false,
            creatingNewSite: false,
            newSiteName: '',
            newSiteIP: '',
            newSitePort: '9000',
            newModeName: '',
            isValidNewMode: false,
            error: false,
        });
    }

    cancelCreateMode() {
        this.reset();
    }

    getSubmitButton() {
        if (this.state.creatingNewMode) {
            return(
                <Button disabled={!this.state.isValidNewMode} variant="contained" color="primary" onClick={() => this.submitNewMode()}>
                    Submit
                </Button>
            )
        } else {
            return(
                <Button disabled={!this.state.isValidNewMode || this.errorForSites()} variant="contained" color="primary" onClick={() => this.submitNewSiteConfiguration()}>
                    Submit
                </Button>
            )
        }
        
    }

    submitNewSiteConfiguration() {
        if (this.state.creatingNewSite) {
            let newMode;
            if (this.props.connection == "FM") {
                newMode = {
                    'siteName' : this.state.newSiteName,
                    'siteId' : this.state.newSiteId,
                    'siteControllerIP' : this.state.newSiteIP,
                    "siteControllerPort" : this.state.newSitePort
                };
            } else if (this.props.connection == "SC" && this.state.newSitePort != "") {
                newMode = {
                    'siteName' : this.state.newSiteName,
                    'siteId' : this.state.newSiteId,

                    "siteControllerPort" : this.state.newSitePort
                };
            } else {
                newMode = {
                    'siteName' : this.state.newSiteName,
                    'siteId' : this.state.newSiteId,

                };
            }
            this.props.submitNewSite(newMode);
        }
        
        this.reset();
    }

    submitNewMode() {
        if (this.state.creatingNewMode) {
            let newName = this.state.newModeName;
            let newMode = {
                [newName]: {
                "color_code": '#D1D1D1',
                "variables": [],
                "constants": []
                }
            };
            
            this.props.submitNewMode(newMode);
        }
        
        this.reset();
    }

    errorForSites() {
        if (this.props.siteData.connection == "FM") {
            return this.errorForSiteNameEmpty()
            || this.errorForSiteNameDuplicate()
            || this.errorForPort()
            || this.errorForIP()
            || this.errorForSiteIdEmpty();
        } else if (this.props.siteData.connection == "CSC") {
            return this.errorForSiteNameEmpty()
            || this.errorForPort();
        } else {
            return this.errorForSiteNameEmpty() || this.errorForSiteIdEmpty();
        }
        
    }

    errorForSiteNameEmpty() {
        return this.state.newSiteName == "";
    }

    errorForSiteIdEmpty() {
        return this.state.newSiteId == "" || this.state.newSiteId.includes(" ");
    }

    errorForSiteNameDuplicate() {
        return false;
    }

    errorForPort() {
        if (this.props.connection == "SC") {
            return false;
        }
        return this.state.newSitePort == "" || isNaN(parseInt(this.state.newSitePort)) || this.state.newSitePort.includes('.');
    }

    errorForIP() {
        if (/^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/.test(this.state.newSiteIP)) {  
            return false; 
        }  
        return true;  
    }

    render() {
        let heightForCreation = '44px';
        if ((this.state.creatingNewSite && this.props.connection == "SC")) {
            heightForCreation = '240px';
        } else if (this.state.creatingNewMode) {
            heightForCreation = '152px';
        }
        else if (this.state.creatingNewSite && this.props.connection == "FM") {
            heightForCreation = '250px';
        }
        return (
            <>
            <div style={{ display: 'flex', flexDirection: 'column', height: '100%', width: '100%'}}>
                <div style={{ flexGrow: 1, width: '100%', overflowY: 'scroll' }}>
                    <div style={{ height: '5%', display: 'flex', justifyContent: 'center', alignItems: 'center' }}>
                        <Typography style={{ fontWeight: 'bold', fontSize: '16px' }}>
                            {this.props.connection == "" && 'Site(s)'}
                            {(this.props.connection == "SC") && 'Site'}
                            {this.props.connection == "FM" && 'Sites'}
                        </Typography>
                    </div> 
                    {this.getSiteButtons()}
                    {(this.props.connection == "FM" || (this.props.connection == "SC" && this.props.siteData.siteControllerPort == null)) &&
                    
                    <div style={{ height: '5%', display: 'flex', justifyContent: 'center', alignItems: 'center' }}>
                        <Typography style={{ fontWeight: 'bold', fontSize: '16px' }}>
                            Modes
                        </Typography>
                    </div>
                    }
                    {(this.props.connection == "FM" || (this.props.connection == "SC" && this.props.siteData.siteControllerPort == null)) && this.getModeButtons()}
                </div>
                <div style={{height: `${heightForCreation}`, display: 'flex', alignContent: 'space-between', flexDirection: 'column'}}>
                    {this.state.creatingNewMode && 
                        <div style={{display: 'flex', justifyContent: 'center', alignItems: 'center'}}>
                            {this.getAddModeTextField()}
                        </div>}
                    {this.state.creatingNewSite && 
                        <div style={{display: 'flex', justifyContent: 'center', alignItems: 'center'}}>
                            {this.getAddSiteTextFields()}
                        </div>}

                    <div style={{  height: '44px', display: 'flex', justifyContent: 'center', alignItems: 'center' }}>
                        {!this.state.creatingNewSite && !this.state.creatingNewMode && this.getSiteAddButton()}
                        {!this.state.creatingNewMode && !this.state.creatingNewSite && (this.props.connection == "FM" || (this.props.connection == "SC" && this.props.siteData.siteControllerPort == null)) && this.getModeAddButton()}
                        {(this.state.creatingNewMode || this.state.creatingNewSite) && this.getCancelButton()}
                        {(this.state.creatingNewMode || this.state.creatingNewSite) && this.getSubmitButton()}
                    </div>
                </div>
            </div>
            </>
        );
    }
}

export default withStyles(ModeList, STYLES_SCHEDULER_CONFIGURATION);