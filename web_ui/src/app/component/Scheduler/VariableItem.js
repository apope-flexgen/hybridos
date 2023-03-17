import React from 'react';
import TextField from '@mui/material/TextField'
import UndoIcon from '@mui/icons-material/Undo';
import IconButton from '@mui/material/IconButton';
import Select from '@mui/material/Select';
import InputLabel from '@mui/material/InputLabel';
import MenuItem from '@mui/material/MenuItem';
import FormControl from '@mui/material/FormControl';
import FormHelperText from '@mui/material/FormHelperText';


class VariableItem extends React.Component {
    constructor(props) {
        super(props);
        
        this.state = {
            value: this.props.value
        }

        this.handleTextChange = this.handleTextChange.bind(this);
        this.handleResetButton = this.handleResetButton.bind(this);
        this.handleDropDown = this.handleDropDown.bind(this);
    }

    componentDidUpdate(prevProps) {
        if (this.props.value !== prevProps.value) {
            this.setState({
                value: this.props.value
            });
        }
    }
    
    handleTextChange(event) {
        if (this.props.field == 'unit') {

        } else if (event.target.value==='' && this.state.value!=='') {
            this.props.handleVariableItemError(1);
        } else if (event.target.value!=='' && this.state.value==='') {
            this.props.handleVariableItemError(-1);
        }
        this.props.handleVariableItemChange(this.props.field, event.target.value);
        this.setState({
            value: event.target.value
        });
    }


    handleDropDown(event) {
        if(event.target.value==='' && this.state.value!=='') this.props.handleVariableItemError(1);
        if(event.target.value!=='' && this.state.value==='') this.props.handleVariableItemError(-1);
        this.props.handleVariableItemChange(this.props.field, event.target.value);
        this.setState({
            value: event.target.value
        })
    }

    handleResetButton(event) {
        if (this.props.field == 'unit') {

        } else if (this.state.value!=='' && this.props.originalValue==='') {
            this.props.handleVariableItemError(1);
        } else if (this.state.value==='' && this.props.originalValue!=='') {
            this.props.handleVariableItemError(-1);
        }
        this.props.handleVariableItemChange(this.props.field, this.props.originalValue);
        this.setState({
            value: this.props.originalValue
        })
    }

    render() {
        let changed = this.props.originalValue!==this.state.value;
        let isEmpty = this.state.value==='';
        return <>
            {this.props.field==="type" ? 
            <FormControl 
            style={{ width: '215px', height: '50%' }} 
            variant="outlined">
                <InputLabel>type</InputLabel>
                <Select
                error={isEmpty}
                label="type"
                onChange={this.handleDropDown}
                value={this.state.value}
                >
                    <MenuItem value="Float">Float</MenuItem>
                    <MenuItem value="Bool">Boolean</MenuItem>
                    <MenuItem value="String">String</MenuItem>
                    <MenuItem value="Int">Integer</MenuItem>
                </Select>
                {isEmpty &&
                <FormHelperText style={{color:'red'}}>This field is required</FormHelperText>
                }
            </FormControl> :
            this.props.field==="unit" ?
            <TextField
                style={{  width: '215px'}}
                label={this.props.field}
                variant="outlined"
                size='small'
                onChange={this.handleTextChange}
                value={this.state.value}
            />
            : 
            <TextField
                style={{  width: '215px'}}
                label={this.props.field}
                variant="outlined"
                size='small'
                error={isEmpty}
                helperText={isEmpty ? 'This field is required' : ''}
                onChange={this.handleTextChange}
                value={this.state.value}
            />
            }
            <IconButton
                onClick={this.handleResetButton}
                disabled={!changed}
                style={{color: `${changed ? '#3F51B5' : '#d1d1d1'}`}}
                size="large"><UndoIcon/></IconButton>
         </>;
    }
}

export default VariableItem;