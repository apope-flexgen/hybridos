import React from 'react';
import IconButton from '@mui/material/IconButton';
import EditIcon from '@mui/icons-material/Edit';
import Confirm from '../component/Confirm';
import { withStyles } from 'tss-react/mui';
import { capitalizeFirst } from '../helpers';
import { STYLES_EVENTS } from '../styles';
import { setPopupAlert } from '../../actions/index';
import { connect } from 'react-redux';
import {
    FormControl,
    InputLabel,
    Select,
    Switch,
    MenuItem,
} from '@mui/material';
import {
    getDataForURI,
    setOrPutDataForURI,
} from '../../AppConfig';

class VariableOverride extends React.Component {
    constructor(props) {
        super(props);

        this.state = {
            selectedSiteName: null,
            selectedIndex: null,
            variableNames: [],
            variableObjects: [],
            isLoading: false,
            siteNames: [],
            readyEdit: [],
            overrideStatuses: [],
            currentValues: [],
            standardValues: [],
            currentOverrides: []
        }
    }

     componentDidMount(){
        this.setState({ isLoading: true });
        getDataForURI('site-admin/read/summary')
        .then((response) => response.json())
        .then((response) => {
            this.setState({
                siteUsesMfa: response.message.password.multi_factor_authentication,
                isLoading: false
            })
            this.fetchSiteNames()
        })
        .catch((error) => {
            console.log(`ERROR in VARIABLE_OVERRIDE/componentDidMount: ${error}`);
        });
     }

     handleSelectSiteName = (event) => {
        if (event.target.value !== 'None'){
            this.setState({selectedSiteName: event.target.value})
            
            this.fetchVariables(event.target.value)
            
            const editModeFlags = {}
            {this.state.variableNames.map((variable, index) => (
                editModeFlags[variable] = false
            ))} 

            this.setState({
                selectedIndex: event.target.index,
                readyEdit: [{}]
            });
        } 
        else {
            this.setState({
                selectedSiteName: 'None',
                selectedIndex: null,
                variableNames: [],
                variableObjects: [],
                readyEdit: [{}]
            });
        }
    }

    fetchSelectValue = (selectedSiteName, variableName, index) => {
        var selectURI = `fleet/features/ercotAs/${selectedSiteName}/${variableName}_select`
        getDataForURI(selectURI)
        .then((response) => response.json())
        .then((response) => {    
            var tempCurrentValues = this.state.currentValues
            tempCurrentValues[index] = response.value
            this.setState({
                currentValues: tempCurrentValues
            })
        })
        .catch((error) => {
            throw new Error(`ERROR in VARIABLE_OVERRIDES/fetchSelectValue: ${error}`);
        });
    }

    fetchCurrentStandardValues = (selectedSiteName) => {
        const varURI = `fleet/features/ercotAs/${selectedSiteName}/overridable`
        console.log(varURI)
        getDataForURI(varURI)
        .then((response) => response.json())
        .then((response) => {
            console.log(response)
            const values = response

            const variableNames = this.state.variableNames
            const currentValues = []
            const manualOverrides = []
            const ercotStandards = []
            const overrideFlags = []

            variableNames.map((variable)  => {
                currentValues.push(values[`${variable}_select`])
                manualOverrides.push(values[`${variable}_manual`])
                ercotStandards.push(values[`${variable}_actual`])
                overrideFlags.push(values[`${variable}_override`])
            })

            this.setState({
                currentValues: currentValues,
                currentOverrides: manualOverrides,
                standardValues: ercotStandards,
                overrideStatuses: overrideFlags
            })
        })
        .catch((error) => {
            throw new Error(`ERROR in VARIABLE_OVERRIDES/fetchCurrentStandardValues: ${error}`);
        });
    }

    fetchVariables = (selectedSiteName) => {
        const varURI = `fleet/features/ercotAs/overridable`
        console.log(varURI)
        getDataForURI(varURI)
        .then((response) => response.json())
        .then((response) => {
            console.log(response)
            const variableNames = response
            this.setState({
                variableNames: variableNames,
            })
            this.fetchCurrentStandardValues(selectedSiteName)
        })
        .catch((error) => {
            throw new Error(`ERROR in VARIABLE_OVERRIDES/fetchSiteNames: ${error}`);
        });
    }


    fetchSiteNames(){
        getDataForURI('fleet/features/ercotAs/sites')
            .then((response) => response.json())
            .then((response) => {
                const siteNames = response
                this.setState({
                    siteNames: siteNames,
                    selectedIndex: null
                })
            })
            .catch((error) => {
                throw new Error(`ERROR in VARIABLE_OVERRIDES/fetchSiteNames: ${error}`);
            });
    }

    turnOffOverride = (variableName, index) => {
        const uri = 'fleet/features/ercotAs/' + this.state.selectedSiteName + "/" + variableName + "_override"
        var body = JSON.stringify({"value": false})
        setOrPutDataForURI(uri, body, 'post')
        .then((response) => response.json())
        .then((response) => {
            this.fetchSelectValue(this.state.selectedSiteName, variableName, index)
        })
        .catch((error) => {
            throw new Error(`Error sending set: ${error}`);
        });
    }

    turnOnOverride = (variableName, index) => {
        const uri = 'fleet/features/ercotAs/' + this.state.selectedSiteName + "/" + variableName + "_override"
        var body = JSON.stringify({"value": true})
        setOrPutDataForURI(uri, body, 'post')
        .then((response) => {
            this.fetchSelectValue(this.state.selectedSiteName, variableName, index)
        })
        .catch((error) => {
            throw new Error(`Error sending set: ${error}`);
        });
    }

    enableEditOverride(variable){
        var variableOverride = document.getElementById(variable['variableName'] + "_override");
        var currentOverride = variableOverride.innerHTML
        variableOverride.innerHTML = ""
        const overrideInput = document.createElement("input");
        overrideInput.setAttribute("type", "text");
        overrideInput.setAttribute("id", (variable['variableName'] + "_overrideTextInput"))
        overrideInput.defaultValue = currentOverride
        variableOverride.appendChild(overrideInput);
        
        let readyEditArray = this.state.readyEdit
        readyEditArray[variable['variableName']] = true
        this.setState({readyEdit: readyEditArray})
    }

    editOverride = (event, type, variable, index) => {
        var variableOverrideTextBox = document.getElementById((variable + "_overrideTextInput"));
        var oldOverride = variableOverrideTextBox.defaultValue
        var newOverride = variableOverrideTextBox.value
        variableOverrideTextBox.remove();
        var variableOverride = document.getElementById(variable + "_override");
        console.log('standard')
        console.log(this.state.standardValues[index])
        if (this.state.standardValues[index] !== false && this.state.standardValues[index] !== true) {
            if (!isNaN(newOverride))
            {
                if (type === 'confirm') {            
                    const uri = 'fleet/features/ercotAs/' + this.state.selectedSiteName + "/" + variable + "_manual"
                    var body = JSON.stringify({"value": newOverride})
                    setOrPutDataForURI(uri, body, 'post')
                    .then((response) => {
                        console.log(response)
                    })
                    .catch((error) => {
                        throw new Error(`Error sending set: ${error}`);
                    });
        
                    variableOverride.innerHTML = newOverride
        
                    this.fetchSelectValue(this.state.selectedSiteName, variable, index)
                }
                else{
                    variableOverride.innerHTML = oldOverride
                }
            }   
            else 
            {
                variableOverride.innerHTML = oldOverride
                this.props.dispatch(setPopupAlert(400, 'Invalid Input: Override value must be a numerical'))
            }
        }
        else {
            if (newOverride === 'true' || newOverride === 'false')
            {
                if (type === 'confirm') {            
                    const uri = 'fleet/features/ercotAs/' + this.state.selectedSiteName + "/" + variable + "_manual"
                    var body = JSON.stringify({"value": newOverride})
                    setOrPutDataForURI(uri, body, 'post')
                    .then((response) => {
                        console.log(response)
                    })
                    .catch((error) => {
                        throw new Error(`Error sending set: ${error}`);
                    });
        
                    variableOverride.innerHTML = newOverride
        
                    this.fetchSelectValue(this.state.selectedSiteName, variable, index)
                }
                else{
                    variableOverride.innerHTML = oldOverride
                }
            }
            else 
            {
                variableOverride.innerHTML = oldOverride
                this.props.dispatch(setPopupAlert(400, 'Invalid Input: Override value must be a boolean'))
            }  
        }
        
        let readyEditArray = this.state.readyEdit
        readyEditArray[variable] = false
        this.setState({readyEdit: readyEditArray})
    }

    handleOverrideToggle= (event, type, index, variable) => {
        const currentOverrideToggleStatus = this.state.overrideStatuses[index]
        if (!currentOverrideToggleStatus){
            this.turnOnOverride(variable, index)
            
            let tempOverrideStats = this.state.overrideStatuses
            tempOverrideStats[index] = true
            this.setState({
                            overrideStatuses: tempOverrideStats
                        })
        }
        else if (currentOverrideToggleStatus){
            this.turnOffOverride(variable, index)
            
            let tempOverrideStats = this.state.overrideStatuses
            tempOverrideStats[index] = false
            this.setState({
                            overrideStatuses: tempOverrideStats, 
                         })
            
        }
    }
    
    render() {
        const { variableNames, variableObjects, selectedIndex, selectedSiteName , siteNames, readyEdit, currentValues, standardValues, currentOverrides, overrideStatuses, isLoading} = this.state;

        const { classes } = this.props;
        return (
            <div>
            <div>
                <FormControl variant="outlined">
                        <InputLabel>Site Name</InputLabel>
                        <Select
                            id="site-name-selector"
                            label="Site Name"
                            value={selectedSiteName}
                            onChange={this.handleSelectSiteName}
                            style={{ width: '240px' }}
                        >
                            {siteNames.map((item, index) => (
                                <MenuItem id={item} key={index} value={item}>{capitalizeFirst(item)}</MenuItem>
                            ))}
                        </Select>
                    </FormControl>
            </div>
            <br></br>
            <br></br>
            <div>
                <table className={classes.table}>
                    <thead>
                        <tr className={classes.row}>
                            <th className={classes.column}>
                                Name
                            </th>
                            <th className={classes.column}>
                                Current Value
                            </th>
                            <th className={classes.column}>
                                ERCOT Standard
                            </th>
                            <th className={classes.column}>
                                Manual Override
                            </th>
                            <th className={classes.column}>
                                Edit Override
                            </th>
                            <th className={classes.column}>
                                Override On/Off
                            </th>
                        </tr>
                    </thead>
                    {this.state.eventsLoading ? <ActivityIndicator leftMargin={'50%'} topMargin={'25%'} color='#000000'/> : <></>}
                    <tbody>
                        {variableNames.map((variableName, index) => (
                            <tr className={classes.row}>
                                <td id={variableName}>{variableName}</td>
                                <td id={variableName + "_current"}>{`${currentValues[index]}`}</td>
                                <td id={variableName + "_standard"}>{`${standardValues[index]}`}</td>
                                <td className={classes.column} id={variableName + "_override"}>{`${currentOverrides[index]}`}</td>
                                {readyEdit[variableName] ? 
                                    <Confirm id={variableName+"_confirm_button"}  
                                        handleConfirm={(event, type) => this.editOverride(event, type, variableName, index)}
                                    />
                                    :
                                    <></>
                                }
                                {!readyEdit[variableName] ? 
                                   <td className={classes.column}>
                                        <IconButton id={variableName + "_edit_button"} aria-label="edit" onClick={() => { this.enableEditOverride({variableName}) }}>
                                            <EditIcon />
                                        </IconButton>
                                    </td>
                                    :
                                    <></>
                                }
                                    <td className={classes.column}>
                                        <Switch
                                            id={variableName + "_toggle"}
                                            checked={overrideStatuses[index]}
                                            onChange={(event, type) => this.handleOverrideToggle(event, type, index, variableName)}
                                            inputProps={{ 'aria-label': 'controlled' }}
                                        />
                                    </td>
                            </tr>     
                        ))}    
                    </tbody>
                </table>          
            </div>
        </div>
        );
    }
}
export default  withStyles(connect()(VariableOverride), STYLES_EVENTS)