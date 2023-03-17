/* eslint-disable react/prop-types */
import React from 'react';

import {
    Paper,
    Button,
    Typography,
    TextField,
} from '@mui/material';

import { Add } from '@mui/icons-material';

class ConfigCard extends React.Component {
    constructor() {
        super();
        this.state = {
            units: 0,
            addItem: false,
            duplicateItem: false,
        };
    }

    setDefaultState = () => {
        this.setState({
            units: 0,
            addItem: false,
            duplicateItem: false,
        });
    }

    handleSubmit = () => {
        this.props.handleAddItem(
            this.state.units,
            this.state.addItem ? 'add' : 'duplicate',
        );
        this.setDefaultState();
    }

    render() {
        const { handleSubmit, setDefaultState } = this;
        const { selectedPage, selectedItem, nameOfItem, retrievingItems } = this.props;
        const { units, addItem, duplicateItem } = this.state;
        // TODO: Change for duplicate (needs an item to be selected)
        const disabled = selectedPage === 'none' || retrievingItems;

        return (
            <Paper style={{ position: 'relative', height: '35%', width: '100%', padding: '10px' }} elevation={3}>
                <div style={{ display: 'flex', width: '100%', justifyContent: 'space-between', marginBottom: '20px' }}>
                    <Button
                        id = 'add-item-button'
                        variant={addItem ? 'contained' : 'outlined'}
                        color="primary"
                        style={{ minHeight: '36px', minWidth: '120px' }}
                        disabled={disabled}
                        onClick={() => this.setState({ addItem: !addItem, duplicateItem: false, units: 0 })}
                    >
                        <Add style={{ marginRight: '5px' }} />
                        {nameOfItem}
                    </Button>
                    <Button
                        variant={duplicateItem ? 'contained' : 'outlined'}
                        color="primary"
                        style={{ minHeight: '36px', minWidth: '120px' }}
                        disabled={disabled || !selectedItem }
                        onClick={() => this.setState({ duplicateItem: !duplicateItem, addItem: false, units: 0 })}
                    >
                        Duplicate
                    </Button>
                </div>
                <div style={{ display: 'flex', width: '100%', justifyContent: 'space-between', alignItems: 'center', marginBottom: '20px' }}>
                    <Typography style={{ opacity: (!addItem && !duplicateItem) ? '0.6' : '1' }}>
                        How many units to add?
                    </Typography>
                    <TextField
                        id = 'num-units'
                        value={units}
                        onChange={(event) => this.setState({ units: event.target.value < 0 ? 0 : event.target.value })}
                        onFocus={(event) => event.target.select() }
                        style={{ width: '80px' }}
                        disabled={!addItem && !duplicateItem}
                        type="number"
                    />
                </div>
                <div style={{ display: 'flex', width: '96%', justifyContent: 'space-between', position: 'absolute', bottom: '10px' }}>
                    <Button
                        variant="outlined"
                        color="primary"
                        style={{ minHeight: '36px', minWidth: '120px' }}
                        disabled={units < 1}
                        onClick={setDefaultState}
                    >
                        Cancel
                    </Button>
                    <Button
                        id = "submit-add-item"
                        variant="contained"
                        color="primary"
                        style={{ minHeight: '36px', minWidth: '120px' }}
                        disabled={units < 1}
                        onClick={handleSubmit}
                    >
                        Submit
                    </Button>
                </div>
            </Paper>
        );
    }
}

export default ConfigCard;
