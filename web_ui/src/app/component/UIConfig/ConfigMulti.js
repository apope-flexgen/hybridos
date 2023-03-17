import React from 'react';

import {
    ListItem,
    ListItemText,
    TextField,
    IconButton,
} from '@mui/material';

import {
    Delete,
} from '@mui/icons-material';

import { capitalizeFirst } from '../../helpers';

import Confirm from '../Confirm';

class ConfigMulti extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            readyDelete: false,
        }
    }

    handleReadyDelete = () => {
        this.setState({ readyDelete: true });
    }

    handleConfirm = (event, type) => {
        if (type === 'confirm') this.props.handleMultiEdit('delete', this.props.index);

        this.setState({ readyDelete: false });
    }

    render() {
        const { newChildValue, index, newValue, handleMulti } = this.props;
        const { readyDelete } = this.state;
        const { handleReadyDelete, handleConfirm } = this;

        return (
            <ListItem style={{ paddingLeft: '40px' }}>
                {Object.keys(newChildValue).map((childKey) => (
                    <div style={{ display: 'flex', marginRight: '20px' }}>
                        <ListItemText style={{ marginRight: '10px' }}>{capitalizeFirst(childKey)}</ListItemText>
                        <form noValidate autoComplete="off">
                            <TextField
                                variant="outlined"
                                size="small"
                                value={newValue[index][childKey]}
                                onChange={(event) => handleMulti(index, childKey, event)}
                                inputProps={{ spellCheck: 'false' }}
                            />
                        </form>
                    </div>
                ))}
                <div style={{ width: '30%' }}>
                    <IconButton onClick={handleReadyDelete} size="large">
                        <Delete />
                    </IconButton>
                    <Confirm
                        disabled={!readyDelete}
                        handleConfirm={handleConfirm}
                    />
                </div>
            </ListItem>
        );
    }
}

export default ConfigMulti;
