/* eslint-disable react/prop-types */
import React from 'react';

import {
    Button,
    List,
    ListItem,
} from '@mui/material';

import ConfigInput from './ConfigInput';
import ConfigCollapsable from './ConfigCollapsable';

import AddItem from '../AddItem';
import Confirm from '../Confirm';

class ConfigTab extends React.Component {
    constructor() {
        super();
        this.state = {
            readyDelete: false
        }
    }

    handleAddField = () => {
        let fieldsCopy = [...this.props.selectedItem[this.props.tab]];
        let newObject = {};
        for (const [key, value] of Object.entries(this.props.inputTypes)) {
            if (value === 'string') newObject[key] = key === 'name' ? `New ${this.props.tab}` : '';
            else newObject[key] = null;
        };
        fieldsCopy.push(newObject);
        this.props.handleChanges(this.props.tab, undefined, fieldsCopy);
    }

    handleDeleteField = (index) => {
        let fieldsCopy = [...this.props.selectedItem[this.props.tab]];
        fieldsCopy.splice(index, 1);
        this.props.handleChanges(this.props.tab, undefined, fieldsCopy);
    }

    handleConfirm = (event, type) => {
        if (type === 'confirm') this.props.handleDeleteItem();
        this.setState({ readyDelete: false });
    }

    render() {
        const { readyDelete } = this.state;
        const { tab, fields, inputTypes, selectedItem, handleChanges } = this.props;
        const { handleAddField, handleDeleteField, handleConfirm } = this;
        const isTemplate = selectedItem && selectedItem.info.isTemplate;
        const batteryView = selectedItem && selectedItem.info.batteryView;
        return (
            <React.Fragment>
                <List>
                    { Array.isArray(fields)
                        ? <React.Fragment>
                            { fields && fields.map((field, index) => (
                                <ConfigCollapsable
                                    fields={field}
                                    inputTypes={inputTypes}
                                    key={index}
                                    index={index}
                                    selectedItem={selectedItem}
                                    handleChanges={(field, newValue) => handleChanges(tab, field, newValue, index)}
                                    handleDeleteField={handleDeleteField}
                                />
                            ))}
                            <AddItem
                                text={`Add ${tab}`}
                                handleAdd={handleAddField}
                            />
                        </React.Fragment>
                        : <React.Fragment>
                            {fields && tab === 'info'
                                && <ListItem
                                    style={{ width: '100%', display: 'flex', flexDirection: 'row', alignItems: 'center', justifyContent: 'flex-end' }}
                                >
                                    <Button
                                        variant={readyDelete ? 'contained' : 'outlined'}
                                        color="secondary"
                                        style={{ minHeight: '42px', minWidth: '120px', marginRight: '10px' }}
                                        onClick={() => this.setState({ readyDelete: !readyDelete })}
                                    >
                                        Delete
                                    </Button>
                                    <Confirm
                                        disabled={!readyDelete}
                                        handleConfirm={handleConfirm}
                                    />
                                </ListItem>
                            }
                            {fields && Object.keys(fields).map((field, index) => (
                                <React.Fragment>
                                    {(!isTemplate || field !== 'baseURI') && (isTemplate || field !== 'items') && (isTemplate || field !== 'batteryView') && (batteryView && isTemplate || field !== 'batteryViewURI')  && (batteryView && isTemplate || field !== 'batteryViewSourceURI')
                                        && <ConfigInput
                                            layoutKeys={this.props.layoutKeys}
                                            field={field}
                                            id={field}
                                            value={fields[field]}
                                            index={index}
                                            inputType={inputTypes[field]}
                                            key={index}
                                            isList={false}
                                            handleChanges={(field, newValue) => handleChanges(tab, field, newValue)}
                                            selectedItem={selectedItem}
                                        />
                                    }
                                </React.Fragment>
                            ))}
                        </React.Fragment>
                    }
                </List>
            </React.Fragment>
        );
    }
}

export default ConfigTab;
