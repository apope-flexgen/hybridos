import React from 'react';

import {
    List,
    ListItem,
    ListItemText,
    Collapse,
    IconButton,
} from '@mui/material';

import {
    ExpandLess,
    ExpandMore,
    Delete,
} from '@mui/icons-material';

import ConfigInput from './ConfigInput';
import Confirm from '../Confirm';

class ConfigCollapsable extends React.Component {
    constructor() {
        super();
        this.state = {
            open: false,
            readyDelete: false,
        };
    }

    handleClick = () => {
        this.setState({ open: !this.state.open });
    }

    handleReadyDelete = (event) => {
        event.stopPropagation();
        this.setState({ readyDelete: true });
    }

    handleConfirm = (event, type) => {
        event.stopPropagation();
        if (type === 'confirm') this.props.handleDeleteField(this.props.index);

        this.setState({ readyDelete: false, open: false });
    }

    render() {
        const { open, readyDelete } = this.state;
        const { handleClick, handleConfirm, handleReadyDelete } = this;
        const { fields, inputTypes, handleChanges, selectedItem } = this.props;

        return (
            <React.Fragment>
                <ListItem button onClick={handleClick}>
                    <ListItemText>
                        {fields.name}
                    </ListItemText>
                    <IconButton onClick={(event) => handleReadyDelete(event)} size="large">
                        <Delete />
                    </IconButton>
                    <Confirm
                        disabled={!readyDelete}
                        handleConfirm={handleConfirm}
                    />
                    {open ? <ExpandLess /> : <ExpandMore />}
                </ListItem>
                <Collapse in={open} timeout="auto" unmountOnExit>
                    <List component="div">
                        {Object.keys(fields).map((field, index) => (
                            <ListItem key={index} style={{ paddingLeft: '40px' }}>
                                <ConfigInput
                                    field={field}
                                    value={fields[field]}
                                    index={index}
                                    inputType={inputTypes[field]}
                                    key={index}
                                    isList={false}
                                    handleChanges={handleChanges}
                                    selectedItem={selectedItem}
                                />
                            </ListItem>
                        ))}
                    </List>
                </Collapse>
            </React.Fragment>
        );
    }
}

export default ConfigCollapsable;
