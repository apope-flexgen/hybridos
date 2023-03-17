import React from 'react';
import Button from '@mui/material/Button';

import { withStyles } from 'tss-react/mui';
import { STYLES_SCHEDULER_CONFIGURATION } from '../../styles';

import VariableItemContainer from './VariableItemContainer';

class VariableList extends React.Component {
    constructor(props) {
        super(props);
        this.state  = {
            open: false,
            elementTitle: null,
        }
        this.deleteVariable = this.deleteVariable.bind(this);
        this.addVariable = this.addVariable.bind(this);
        this.handleVariableItemContainerChange = this.handleVariableItemContainerChange.bind(this);
        this.handleVariableItemContainerError = this.handleVariableItemContainerError.bind(this);
    }

    handleVariableItemContainerError(error) {
        this.props.handleVariableListError(error);
    }

    handleVariableItemContainerChange(variableCopy, index) {
        let newMode = JSON.parse(JSON.stringify(this.props.updatingMode));
        let modeTitle = Object.keys(this.props.updatingMode)[0];
        newMode[modeTitle][this.props.isConstant ? 'constants' : 'variables'][index] = variableCopy;
        this.props.updateUpdatingModeWith(newMode);
        
    }
    generateList() {
        let modeTitle = Object.keys(this.props.updatingMode)[0];
        let originalVariablesAndConstants = this.props.selectedMode[modeTitle];
        let variablesAndConstants = this.props.updatingMode[modeTitle];
        let type = this.props.isConstant ? 'constants' : 'variables';
        let variables = variablesAndConstants[type];
        let originalVariables = originalVariablesAndConstants[type];
       
        let listItems = [];
        if(variables==null) variables=[];
        for(let i=0; i<variables.length; i++) {
            let emptyData = {id: '', name: '', uri: '', type: '', unit: '', value:''};
            listItems.push(
                <VariableItemContainer
                    handleVariableItemContainerError={this.handleVariableItemContainerError}
                    index={i}
                    handleVariableItemContainerChange={this.handleVariableItemContainerChange}
                    deleteVariable={this.deleteVariable}
                    variable={this.props.updatingMode[modeTitle][type][i]}
                    originalVariable={(i>=originalVariables.length) ? emptyData : this.props.selectedMode[modeTitle][type][i]}
                    isConstant={this.props.isConstant}
                />
            );

        }
        return listItems;
    }
    deleteVariable(variableData) {
        let newUpdatingMode = JSON.parse(JSON.stringify(this.props.updatingMode));
        let newSelectedModeWithDeletions = JSON.parse(JSON.stringify(this.props.selectedMode))
        let modeTitle = Object.keys(newUpdatingMode)[0];
        let type = this.props.isConstant ? 'constants' : 'variables';
        let varList = newUpdatingMode[modeTitle][type];
        
        let secondVarList = newSelectedModeWithDeletions[modeTitle][type];
        let index = -1;
        for (let i = 0; i < varList.length; i++) {
            if (varList[i].id == variableData.id) {
                index = i
            }
        }
        let numErrors = 0;
        for (let i = 0; i < Object.keys(varList[index]).length; i++) {
            if (Object.keys([index])[i] == 'unit') {
            } else if (varList[index][Object.keys(varList[index])[i]] == '') {
                numErrors -= 1;
            }
        }
        this.props.handleVariableListError(numErrors)
        varList.splice(index, 1);
        secondVarList.splice(index, 1);

        this.props.updateUpdatingModeWith(newUpdatingMode, newSelectedModeWithDeletions)
    }

    addVariable() {
        let modeTitle = Object.keys(this.props.updatingMode)[0];
        let type = this.props.isConstant ? 'constants' : 'variables';
        let varList = this.props.updatingMode[modeTitle][type];
        let newVar = {
            id: '',
            name: '',
            type: '',
            unit: '',
            uri: '',
            value: '',
        }
        varList.push(newVar);
        this.props.handleVariableListError(5);
        this.props.updateUpdatingModeWith(JSON.parse(JSON.stringify(this.props.updatingMode)))

    }

    render() {
        let type = this.props.isConstant ? 'constants' : 'variables';
        return (
            <>
                <div style={{position: 'relative', height:'90%', width: '100%', overflowY: 'scroll'}}>
                    {this.generateList()}
                </div>
                <div style={{ height: '10%', display: 'flex', justifyContent: 'center', alignItems: 'center' }}>
                    <Button disabled={(Object.keys(this.props.selectedMode)[0] == 'default' && !this.props.isConstant) || (this.props.selectedMode[Object.keys(this.props.selectedMode)[0]][type] == null)} onClick={this.addVariable} variant='contained' color='primary'>ADD {this.props.isConstant ? 'CONSTANT' : 'VARIABLE'}</Button>
                </div>
            </>
        );
    }
}

export default withStyles(VariableList, STYLES_SCHEDULER_CONFIGURATION);