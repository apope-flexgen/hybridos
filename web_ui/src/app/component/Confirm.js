import React from 'react';

import {
    IconButton,
} from '@mui/material';

import {
    Check,
    Clear,
} from '@mui/icons-material';

function Confirm(props) {
    const { handleConfirm } = props;

    return (
        <div>
            <IconButton
                identifier="check-button"
                color='primary'
                disabled={props.disabled}
                onClick={(event) => handleConfirm(event, 'confirm')}
                size="large">
                <Check />
            </IconButton>
            <IconButton
                color='secondary'
                disabled={props.disabled}
                onClick={(event) => handleConfirm(event, 'cancel')}
                size="large">
                <Clear />
            </IconButton>
        </div>
    );
}

export default Confirm;