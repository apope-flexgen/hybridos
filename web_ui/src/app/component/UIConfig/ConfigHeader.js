/* eslint-disable react/prop-types */
import React from 'react';

import {
    FormControl,
    InputLabel,
    FormHelperText,
    Select,
    MenuItem,
    Button,
    Box,
} from '@mui/material';

import { capitalizeFirst } from '../../helpers';

// import {
//     Visibility,
//     VisibilityOff,
// } from '@mui/icons-material';

import Confirm from '../Confirm';

class ConfigHeader extends React.Component {
    constructor() {
        super();
        this.state = {
            save: false,
            cancel: false,
        };
    }

    handleConfirm = (event, type) => {
        if (type === 'confirm') this.props.handleSave(this.state.save);
        this.setState({
            save: false,
            cancel: false,
        })
    }

    render() {
        const { save, cancel } = this.state;
        const { selectedPage, pages, handlePageSelect, hasChanged, handlePageSave } = this.props;
        const { handleConfirm } = this;

        return (
            <Box style={{ display: 'flex', justifyContent: 'space-between',  flexGrow: 1 }}>
                <div className="Page-Selector" style={{ marginBottom: '10px' }}>
                    <FormControl variant="outlined">
                        <InputLabel>Page</InputLabel>
                        <Select
                            id = "page-selector-id"
                            label="Page"
                            value={selectedPage}
                            onChange={handlePageSelect}
                            style={{ width: '200px' }}
                        >
                            {Object.keys(pages).map((page, index) => {
                                if (page !== 'version') return <MenuItem key={index} id={page} value={page}>{capitalizeFirst(page)}</MenuItem>
                            })}
                        </Select>
                        <FormHelperText>Select page to configure</FormHelperText>
                    </FormControl>
                    {/* TODO: Figure out logic and purpose */}
                    {/* <IconButton
                        onClick={handleShowPage}
                        style={{ color: 'black', marginLeft: '10px' }}
                    >
                        {this.state.showPage ? <Visibility /> : <VisibilityOff />}
                    </IconButton> */}
                </div>
                <Box style={{ display: 'flex', height: '50px' }}>
                    <Button
                        variant={cancel ? 'contained' : 'outlined'}
                        color="primary"
                        style={{ minHeight: '36px', minWidth: '120px' }}
                        disabled={!hasChanged}
                        onClick={() => this.setState({ cancel: !cancel, save: false })}     
                    >
                        Cancel
                    </Button>
                    <Button
                        id='ui-config-save'
                        variant={save ? 'contained' : 'outlined'}
                        color="primary"
                        style={{ minHeight: '36px', minWidth: '120px', marginLeft: '20px', marginRight: '10px' }}
                        disabled={!hasChanged}
                        onClick={() => this.setState({ save: !save, cancel: false })}
                    >
                        Save
                    </Button>
                    <Confirm
                        disabled={!save && !cancel}
                        handleConfirm={handleConfirm}
                    />
                </Box>
            </Box>
        );
    }
}

export default ConfigHeader;
