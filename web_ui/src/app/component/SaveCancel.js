import React, { useState } from 'react';

import {
    Box,
    Button
} from '@mui/material';

import Confirm from './Confirm';

function SaveCancel(props) {
    const [save, setSave] = useState(false);
    const [cancel, setCancel] = useState(false);
    
    return (
        <Box style={{ display: 'flex', flexDirection: 'row', height: '50px' }}>
            <Button
                id="cancel-button"
                variant={cancel ? 'contained' : 'outlined'}
                color="primary"
                style={{ minHeight: '36px', minWidth: '120px' }}
                disabled={props.disabled}
                onClick={() => {
                    setSave(false);
                    setCancel(true);
                }}     
            >
                Cancel
            </Button>
            <Button
                id="save-button"
                variant={save ? 'contained' : 'outlined'}
                color="primary"
                style={{ minHeight: '36px', minWidth: '120px', marginLeft: '20px', marginRight: '10px' }}
                disabled={props.disabled}
                onClick={() => {
                    setSave(true);
                    setCancel(false);
                }}     
            >
                Save
            </Button>
            <Confirm
                disabled={!save && !cancel}
                handleConfirm={(event, type) => {
                    if (type === 'confirm') {
                        setSave(false);
                        setCancel(false);
                        props.handleSave(save);
                    } else {
                        setSave(false);
                        setCancel(false);
                    }
                }}
            />
        </Box>
    );
}

export default SaveCancel;