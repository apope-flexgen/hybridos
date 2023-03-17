import React from 'react';
import { withStyles } from 'tss-react/mui';

import {
    Box,
    FormControl,
    InputLabel,
    FormHelperText,
    Select,
    MenuItem,
    Button,
    Paper,
    List,
    ListSubheader,
    ListItem,
    ListItemText,
} from '@mui/material';

import {
    getDataForURI,
    setOrPutDataForURI,
} from '../../AppConfig';

import {
    createUser,
    removeUser,
    editUser,
    userRole
} from '../../AppAuth';

import {
    Add,
    LeakAdd
} from '@mui/icons-material';

import { STYLES_USER_ADMIN } from '../styles';
import { capitalizeFirst } from '../helpers';

import Loading from '../component/Loading';
import SaveCancel from '../component/SaveCancel';
import Confirm from '../component/Confirm';
import { TextInput, ButtonInput, SwitchInput, NumberInput, ProgressButtonInput } from '../component/Inputs';

import { setPopupAlert } from '../../actions/index';
import { connect } from 'react-redux';

const CURRENT_VERSION = '1.0.0';

const SETTINGS = [
    'none',
    'password',
    'radius'
]

const LAYOUT = {
    "version": CURRENT_VERSION,
    "password": {
        "settings": [
            {
                "name": "Minimum password length",
                "key": "minimum_password_length",
                "type": "number",
            },
            {
                "name": "Maximum length",
                "key": "maximum_password_length",
                "type": "number",
            },
            {
                "name": "Custom regex",
                "key": "password_regular_expression",
                "type": "string",
            },
            {
                "name": "Multi Factor Authentication",
                "key": "multi_factor_authentication",
                "type": "boolean"
            },
            {
                "name": "Number of old passwords allowed",
                "key": "old_passwords",
                "type": "number",
                "helperText": "To disable, enter 0 in the field"
            },
            {
                "name": "Allow password expiration",
                "key": "password_expiration",
                "type": "boolean"
            },
            {
                "name": "Interval for password expiration",
                "key": "password_expiration_interval",
                "type": "string",
                "helperText": "Number followed by d (days) or m (minutes) e.g. 8d for 8 days"
            },
        ],
    },
    "radius": {
        "settings": [
            {
                "name": "Enabled",
                "key": "is_enabled",
                "type": "radius_enable",
                "helperText": "Test Required"
            },
            {
                "name": "Send Test Request",
                "key": "none",
                "type": "radius_test",
                "helperText": "Press button to send a request to the configured server"
            },
            {
                "name": "Username",
                "key": "_username",
                "type": "string",
            },
            {
                "name": "Password",
                "key": "_password",
                "type": "password",
            },
            {
                "name": "IP Address",
                "key": "ip_address",
                "type": "string"
            },
            {
                "name": "Port",
                "key": "port",
                "type": "number"
            },
            {
                "name": "Secret Phrase",
                "key": "secret_phrase",
                "type": "password"
            },
            {
                "name": "Wait Time",
                "key": "wait_time",
                "type": "number",
                "helperText": "How long a request should wait (in ms) for a response back"
            },
        ]
    }
}

class UserAdministration extends React.Component {
    constructor() {
        super();

        this.state = {
            selectedSetting: 'none',
            isLoading: false,
            showAlert: false,
            values: null,
            originalValues: null,
            edited: false,
            // Special Settings
            // Radius Settings
            radiusTestSuccess: false,
            radiusTestLoading: false,
        }
    }

    componentDidMount() {
        this.setState({ isLoading: true });
        getDataForURI('site-admin/read/summary')
            .then((response) => response.json())
            .then((response) => {
                let values = response.message;
                // Prevents copying error, might have to add more when expanding this page
                values.radius._username = '';
                values.radius._password = '';
                this.setState({
                    values,
                    originalValues: JSON.parse(JSON.stringify(values)),
                    isLoading: false
                });
            })
            .catch((error) => {
                console.log(`ERROR in SITE_ADMINISTRATION/componentDidMount: ${error}`);
            })
    }

    handleSelectSetting = (event) => {
        if (event.target.value !== 'none') {
            this.setState({
                selectedSetting: event.target.value
            });
        }
        else {
            this.setState({
                selectedSetting: 'none',
                selectedIndex: null,
                users: []
            });
        }
    }

    handleChange = (event, key, type) => {
        const { selectedSetting } = this.state;
        let valuesCopy = { ...this.state.values };
        let edited = this.state.edited;
        let radiusTestSuccess = this.state.radiusTestSuccess
        switch (type) {
            case 'radius_enable':
            case 'boolean':
                valuesCopy[selectedSetting][key] = event.target.checked;
                edited = true;
                break;
            case 'password':
            case 'string':
                valuesCopy[selectedSetting][key] = event.target.value;
                edited = true;
                break;
            case 'number':
                const { value } = event.target;
                if (value.match(/^-?\d*\.?\d*$/, '')) valuesCopy[selectedSetting][key] = value.length !== 0 ? parseFloat(value) : value;
                edited = true;
                break;
            case 'radius_test':
                this.tryRadiusConnection();
                break;
            default:
                console.log('Unhandled type: ', type);
                break;
        }

        if (selectedSetting === 'radius' && !['radius_enable', 'radius_test'].includes(type)) {
            radiusTestSuccess = false;
            valuesCopy[selectedSetting].is_enabled = false;
        }

        this.setState({
            values: valuesCopy,
            edited,
            radiusTestSuccess
        })
    }

    tryRadiusConnection = () => {
        this.setState({
            radiusTestLoading: true,
        });

        fetch('/api/radius_test', {
            method: 'POST',
            credentials: 'include',
            headers: {          
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(this.state.values.radius)
        })
            .then((response) => response.json())
            .then((response) => {
                if (response.status === 200) {

                    this.setState({
                        radiusTestSuccess: true,
                        radiusTestLoading: false,
                    })
                } else {
                    this.setState({ radiusTestLoading: false });
                }
            })
            .catch((error) => {
                console.log(`ERROR in SITE_ADMINISTRATION/handleSave: ${error}`);
            });
    }

    handleSave = (save) => {
        if (save) {
            let baseString = '/api/site-admin/edit';

            fetch(baseString, {
                method: 'POST',
                credentials: 'include',
                headers: {          
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(this.state.values)
            })
                .then((response) => response.json())
                .then((response) => {
                    this.props.dispatch(setPopupAlert(response.status, response.message, true))
                    this.setState({
                        originalValues: this.state.values,
                        edited: false,
                    })
                })
                .catch((error) => {
                    console.log(`ERROR in SITE_ADMINISTRATION/handleSave: ${error}`);
                });
        } else {
            this.setState({
                values: JSON.parse(JSON.stringify(this.state.originalValues)),
                edited: false,
                radiusTestSuccess: false
            })
        }
    }

    render() {
        const { classes } = this.props;
        const { isLoading, selectedIndex, selectedSetting, showAlert, values, originalValues, popupMeta } = this.state;
    
        return (
            <div>
                <Box style={{ display: 'flex', flexDirection: 'row', justifyContent: 'space-between' }}>
                    <FormControl variant="outlined">
                        <InputLabel>Settings</InputLabel>
                        <Select
                            label="Settings"
                            value={selectedSetting}
                            onChange={this.handleSelectSetting}
                            style={{ width: '240px' }}
                        >
                            {SETTINGS.map((item, index) => (
                                <MenuItem key={index} value={item}>{capitalizeFirst(item)}</MenuItem>
                            ))}
                        </Select>
                        <FormHelperText>Select a setting to modify</FormHelperText>
                    </FormControl>
                    <SaveCancel
                        handleSave={this.handleSave}
                        disabled={!this.state.edited}
                    />
                </Box>
                <div> {/* not sure if this wrapping div is needed */}
                    <div style={{ width: '100%', minWidth: '700px', minHeight: '70vh', display: 'flex', flexDirection: 'column' }}>
                        <Paper style={{ height: '100%', flex: '1' }}>
                            <List subheader={<li />}>
                                <ListSubheader className={classes.subheader}>
                                    Settings
                                </ListSubheader>
                                {values && values[selectedSetting] && LAYOUT[selectedSetting] && LAYOUT[selectedSetting].settings.map((setting, index) => (
                                    <ListItem
                                        style={{ display: 'flex', justifyContent: 'space-between', width: '75%' }}
                                    >
                                        <ListItemText>{setting.name}</ListItemText>
                                        {/* Generic setting types */}
                                        {setting.type === 'string' &&
                                            <TextInput
                                                value={values[selectedSetting][setting.key]}
                                                helperText={setting.helperText ? setting.helperText : ''}
                                                handleChange={(event) => this.handleChange(event, setting.key, setting.type)}
                                            />
                                        }
                                        {setting.type === 'password' &&
                                            <TextInput
                                                value={values[selectedSetting][setting.key]}
                                                helperText={setting.helperText ? setting.helperText : ''}
                                                handleChange={(event) => this.handleChange(event, setting.key, setting.type)}
                                                password
                                            />
                                        }
                                        {setting.type === 'boolean' &&
                                            <SwitchInput
                                                value={values[selectedSetting][setting.key]}
                                                handleChange={(event) => this.handleChange(event, setting.key, setting.type)}
                                                label={setting.helperText ? setting.helperText : ''}
                                            />
                                        }
                                        {setting.type === 'number' &&
                                            <NumberInput
                                                value={values[selectedSetting][setting.key]}
                                                helperText={setting.helperText ? setting.helperText : ''}
                                                handleChange={(event) => this.handleChange(event, setting.key, setting.type)}
                                            />
                                        }
                                        {/* Special setting types */}
                                        {/* Radius Settings */}
                                        {setting.type === 'radius_enable' &&
                                            <SwitchInput
                                                value={values[selectedSetting][setting.key]}
                                                handleChange={(event) => this.handleChange(event, setting.key, setting.type)}
                                                disabled={!this.state.radiusTestSuccess && !values[selectedSetting][setting.key]}
                                                label={setting.helperText}
                                            />
                                        }
                                        {setting.type === 'radius_test' &&
                                            <ProgressButtonInput
                                                handleChange={() => this.handleChange(null, setting.key, setting.type)}
                                                icon={<LeakAdd />}
                                                loading={this.state.radiusTestLoading}
                                                success={this.state.radiusTestSuccess}
                                            />
                                        }
                                    </ListItem>
                                ))}
                            </List>
                        </Paper>
                    </div>
                </div>
            </div>
        );
    }
}

export default withStyles(connect()(UserAdministration), STYLES_USER_ADMIN);