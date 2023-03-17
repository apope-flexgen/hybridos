/* eslint-disable react/prop-types */
import React from 'react';

import {
    List,
    ListItem,
    ListItemText,
    Collapse,
    FormControl,
    Checkbox,
    TextField,
    Select,
    MenuItem,
    InputLabel,
    Input,
} from '@mui/material';

import {
    ExpandLess,
    ExpandMore,
} from '@mui/icons-material';

import { capitalizeFirst } from '../../helpers';

import ConfigMulti from './ConfigMulti';
import AddItem from '../AddItem';

class ConfigInput extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            newValue: props.value,
            open: false,
            readyDelete: false,
        };
    }

    componentDidUpdate(prevProps) {
        if (this.props.selectedItem !== prevProps.selectedItem) {
          this.setState({
              newValue: this.props.value
          });
        }
    }

    handleClick = () => {
        this.setState({ open: !this.state.open });
    }

    handleChange = (event, type) => {
        let newValue;
        switch (type) {
            case 'checked':
                newValue = event.target.checked;
                break;
            case 'text':
                newValue = event.target.value;
                break;
            case 'number':
                newValue = event.target.value.match(/^-?\d*\.?\d*$/, '') ? event.target.value : this.state.newValue;
                break;
            case 'count':
                newValue = event.target.value < 0 ? 0 : event.target.value;
                break;
            case 'select':
                newValue = event.target.value;
                break;
            case 'array':
                newValue = event.target.value.split(',');
                break;
            default:
                break;
        }
        this.setState({ newValue });
        this.props.handleChanges(this.props.field, newValue);
    }

    handleMulti = (index, key, event) => {
        let valueCopy = [...this.state.newValue];
        valueCopy[index][key] = event.target.value;

        this.setState({ newValue: valueCopy });
        this.props.handleChanges(this.props.field, valueCopy);
    }

    handleMultiEdit = (type, index) => {
        let valueCopy = [...this.state.newValue];
        if (type === 'add') {
            let newObject = {};
            for (const [key, value] of Object.entries(this.props.inputType.other)) {
                let newValue = null;
                switch (value) {
                    case 'string':
                        newValue = '';
                    default:
                        newObject[key] = newValue;
                        break;
                }
            }
            valueCopy.push(newObject);
        }
        if (type === 'delete') {
            valueCopy.splice(index, 1);
        }

        this.setState({ newValue: valueCopy });
        this.props.handleChanges(this.props.field, valueCopy);
    }

    render() {
        const { field, inputType, selectedItem, layoutKeys } = this.props;
        const { newValue, open, readyDelete } = this.state;
        const { handleChange, handleClick, handleMulti, handleMultiEdit } = this;

        // Can add more
        const toggleExpandable = inputType && inputType.type && inputType.type === 'multi';

        return (
            <React.Fragment>
                <ListItem
                    style={{ width: '100%', display: 'flex', flexDirection: 'row', alignItems: 'center' }}
                    button={toggleExpandable}
                    onClick={handleClick}
                >
                    <ListItemText>{capitalizeFirst(field)}</ListItemText>
                    {typeof inputType === 'object'
                        && <React.Fragment>
                            {inputType.type === 'select'
                                && <FormControl variant="outlined">
                                    <Select
                                        value={newValue}
                                        onChange={(event) => handleChange(event, 'select')}
                                        style={{minWidth: 150}}
                                    >
                                        {Array.isArray(inputType.other)
                                            ? inputType.other.map((value, index) => (
                                                <MenuItem key={index} value={value}>{value}</MenuItem>
                                            ))
                                            : selectedItem.info[inputType.other].map((value, index) => (
                                                <MenuItem key={index} value={value}>{value}</MenuItem>
                                            ))
                                        }
                                    </Select>
                                </FormControl>
                            }
                        </React.Fragment>
                    }
                    {inputType === 'array'
                        && <form noValidate autoComplete="off">
                            <TextField
                                variant="outlined"
                                size="small"
                                helperText="Separate inputs with comma"
                                value={newValue.toString()}
                                onChange={(event) => handleChange(event, 'array')}
                                inputProps={{ spellCheck: 'false' }}
                            />
                        </form>
                    }
                    {inputType === 'boolean'
                        && <Checkbox
                            checked={newValue}
                            onChange={(event) => handleChange(event, 'checked')}
                            color="primary"
                        />
                    }
                    {inputType === 'string'
                        && <form noValidate autoComplete="off">
                            <TextField
                                variant="outlined"
                                size="small"
                                value={newValue}
                                onChange={(event) => handleChange(event, 'text')}
                                inputProps={{ spellCheck: 'false' }}
                            />
                        </form>
                    }
                    {inputType === 'layoutKey'
                        && <form noValidate autoComplete="off">
                            <FormControl>
                                <InputLabel id="test-select-label">Select Key</InputLabel>
                                <Select
                                    label="Select Key"
                                    variant="outlined"
                                    labelId="asset-key-select-label"
                                    id="asset-key-select"
                                    value={newValue}
                                    onChange={(event) => handleChange(event, 'text')}
                                    inputProps={{ spellCheck: 'false' }}
                                    style={{minWidth: 150}}
                                >
                                    <MenuItem value="">
                                        <em>None</em>
                                    </MenuItem>
                                    {layoutKeys && layoutKeys.map((layoutKey) => (
                                        <MenuItem
                                            key={layoutKey}
                                            value={layoutKey}
                                        >
                                            {layoutKey}
                                        </MenuItem>
                                    ))}
                                </Select>
                                </FormControl>
                        </form>
                    }
                    {inputType === 'number'
                        && <form noValidate autoComplete="off">
                            <TextField
                                variant="outlined"
                                size="small"
                                value={newValue}
                                onChange={(event) => handleChange(event, 'number')}
                                inputProps={{ spellCheck: 'false' }}
                                helperText="Numbers only"
                            />
                        </form>
                    }
                    {inputType === 'count'
                        && <TextField
                            value={newValue}
                            onChange={(event) => handleChange(event, 'count')}
                            onFocus={(event) => event.target.select() }
                            style={{width: 75}}
                            type="number"
                        />
                    }
                    {toggleExpandable
                        && <React.Fragment>
                            {open ? <ExpandLess /> : <ExpandMore />}
                        </React.Fragment>
                    }
                </ListItem>
                {toggleExpandable
                    && <Collapse in={open} timeout="auto" unmountOnExit>
                        <List component="div">
                            {inputType.type === 'multi' && newValue.map((newChildValue, index) => (
                                <ConfigMulti
                                    newChildValue={newChildValue}
                                    index={index}
                                    key={index}
                                    newValue={newValue}
                                    handleMulti={handleMulti}
                                    handleMultiEdit={handleMultiEdit}
                                />
                            ))}
                            <AddItem
                                text="Add Item"
                                handleAdd={() => handleMultiEdit('add')}
                                styling={{ paddingLeft: '40px' }}
                            />
                        </List>
                    </Collapse>
                }
            </React.Fragment>
        );
    }
}

export default ConfigInput;
