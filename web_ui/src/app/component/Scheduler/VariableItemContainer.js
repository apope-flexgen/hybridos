import React from 'react';
import List from '@mui/material/List';
import ListItemText from '@mui/material/ListItemText';
import ListItem from '@mui/material/ListItem';
import ExpandLess from '@mui/icons-material/ExpandLess';
import ExpandMore from '@mui/icons-material/ExpandMore';
import Collapse from '@mui/material/Collapse';
import IconButton from '@mui/material/IconButton';


import ListItemIcon from '@mui/material/ListItemIcon';
import DeleteIcon from '@mui/icons-material/Delete';
import CheckIcon from '@mui/icons-material/Check';
import ClearIcon from '@mui/icons-material/Clear';

import VariableItem from './VariableItem';


class VariableItemContainer extends React.Component {
    constructor(props) {
        super(props);
        this.state  = {
            open: false,
            deleteHasBeenClicked: false,
        }

        this.handleClick = this.handleClick.bind(this);
        this.handleDeleteClick = this.handleDeleteClick.bind(this);
        this.handleCancelClick = this.handleCancelClick.bind(this);
        this.handleConfirmDelete = this.handleConfirmDelete.bind(this);
        this.generateSublist = this.generateSublist.bind(this);
        this.handleVariableItemChange = this.handleVariableItemChange.bind(this);
        this.handleVariableItemError = this.handleVariableItemError.bind(this)
    }

    componentDidUpdate(prevProps) {
        if (this.props.variable !== prevProps.variable && this.state.deleteHasBeenClicked) {
            this.setState({
                open: false,
                deleteHasBeenClicked: false
            });
        }
    }

    handleVariableItemError(error){
        this.props.handleVariableItemContainerError(error);
    }

    handleVariableItemChange(field, value) {
        let variableCopy = JSON.parse(JSON.stringify(this.props.variable));
        variableCopy[field] = value;
        this.props.handleVariableItemContainerChange(variableCopy, this.props.index);
    }

    handleClick(event) {
        this.setState({
            open: !this.state.open,
        });
    }
    
    isDisabled() {
        return !this.state.deleteHasBeenClicked;
    }
    
    handleDeleteClick(event) {
        event.stopPropagation();
        this.setState({
            deleteHasBeenClicked: true,
           
        });
    }
    handleCancelClick(event) {
        event.stopPropagation();
        this.setState({
            deleteHasBeenClicked: false
        });
    }

    handleConfirmDelete(event) {
        event.stopPropagation();

        this.props.deleteVariable(this.props.variable);
    }

    generateSublist() {

        return (
            <>
                {Object.keys(this.props.variable).map(field => {
                       
                        return (
                            <ListItem>
                                <div style = {{ display: 'flex', alignItems: 'center', flexDirection: 'column' }}>
                                    {field == "value" && !this.props.isConstant &&
                                        <ListItemIcon>
                                            default
                                        </ListItemIcon>
                                    }
                                    <ListItemIcon>
                                        {field}
                                    </ListItemIcon>
                                </div>
                                
                                <VariableItem
                                 handleVariableItemError={this.handleVariableItemError}
                                 handleVariableItemChange={this.handleVariableItemChange}
                                 updateUpdatingModeWith={this.props.updateUpdatingModeWith}
                                 originalValue={this.props.originalVariable[field]}
                                 value={this.props.variable[field]} 
                                 field={field} 
                                />
                            </ListItem>
                        )
                })}                
            </>
        );
    }

    render() {
        return <>
        <ListItem button onClick={this.handleClick} style={{ width: '100%' }}>
            <ListItemText primary={this.props.variable.name}>
            </ListItemText>
            <IconButton onClick={this.handleDeleteClick} style={{ color: 'black'}} size="large"><DeleteIcon/></IconButton>
            
            <IconButton
                onClick={this.handleConfirmDelete}
                disabled={this.isDisabled()}
                style={{color: `${this.state.deleteHasBeenClicked ? '#4BC940' : '#d1d1d1'}`}}
                size="large"><CheckIcon/></IconButton>
            
            <IconButton
                onClick={this.handleCancelClick}
                disabled={this.isDisabled()}
                style={{color: `${this.state.deleteHasBeenClicked ? '#E33B3B' : '#d1d1d1'}`}}
                size="large"><ClearIcon/></IconButton>
            {this.state.open ? <ExpandLess /> : <ExpandMore />}
        </ListItem>
        <Collapse style={{ height: '100%' }} in={this.state.open} timeout="auto" unmountOnExit>
            <List>
                {this.generateSublist()}
            </List>
        </Collapse>
        </>;
    }
}

export default VariableItemContainer;