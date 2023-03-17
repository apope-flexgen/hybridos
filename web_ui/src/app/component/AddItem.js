import React from 'react';

import {
    ListItem,
    ListItemText,
} from '@mui/material';

import {
    Add,
} from '@mui/icons-material';

function AddItem(props) {
    return (
        <ListItem button style={{ ...props.styling }} onClick={props.handleAdd}>
            <ListItemText>{props.text}</ListItemText>
            <Add color="primary" fontSize="large"/>
        </ListItem>
    );
}

export default AddItem;