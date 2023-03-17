/* eslint-disable no-unused-expressions */
/* eslint-disable no-underscore-dangle */
/* eslint-disable camelcase */
/* eslint-disable react/prop-types */
import React from 'react';
import Grid from '@mui/material/Grid';
import IconButton from '@mui/material/IconButton';
import FormControl from '@mui/material/FormControl';
import FormControlLabel from '@mui/material/FormControlLabel';
import FormGroup from '@mui/material/FormGroup';
import Switch from '@mui/material/Switch';
import Button from '@mui/material/Button';
import TextField from '@mui/material/TextField';

import Check from '@mui/icons-material/Check';
import Close from '@mui/icons-material/Close';

import { updatePropertyValue } from '../../../AppConfig';

/**
 * Renders control pane for Grid View
 */
class Control extends React.Component {
    constructor(props) {
        super(props);
        let setupProps;
        switch (props.control.type) {
            case 'enum_slider':
                setupProps = { checked: props.control.value };
                break;
            case 'enum_button':
                setupProps = { pressed: false };
                break;
            case 'number':
                setupProps = { fieldValue: '' };
                this.inputField = React.createRef();
                break;
            default:
                console.log(`ERROR: invalid control type ${props.control.type} supplied`);
        }

        this.state = {
            confirmEnabled: false,
            ...setupProps,
        };

        this.handleSwitch = this.handleSwitch.bind(this);
    }

    componentDidMount() {
        this._isMounted = true;
    }

    componentWillUnmount() {
        this._isMounted = false;
    }

    /**
     * Resets controls between grid selections
     * @param {*} prevProps
     */
    componentDidUpdate(prevProps) {
        // Required to update controls when switching between assets. Otherwise control is sticky
        if (prevProps.control.asset_id !== this.props.control.asset_id) {
            this.resetState();
        }
    }

    /**
     * Sets state back to default
     */
    resetState() {
        let setupProps;
        switch (this.props.control.type) {
            case 'enum_slider':
                setupProps = { checked: this.props.control.value };
                break;
            case 'enum_button':
                setupProps = { pressed: false };
                break;
            case 'number':
                setupProps = { fieldValue: '' };
                break;
            default:
                console.log(`ERROR: invalid control type ${this.props.control.type} supplied`);
        }

        this.setState({
            confirmEnabled: false,
            ...setupProps,
        });
    }

    /**
     * Handles switch logic
     */
    handleSwitch() {
        this.setState({
            confirmEnabled: !this.state.confirmEnabled,
            checked: !this.state.checked,
        });
    }

    /**
     * Handles button logic
     */
    handleButton() {
        this.setState({
            confirmEnabled: true,
            pressed: true,
        });
    }

    /**
     * Handles text field logic
     * @param {*} event
     */
    handleTextField(event) {
        const { value } = event.target;
        const floatRegex = /^-?\d*\.?\d*$/;

        // If next character is '-' do sign-change
        if (value.length - this.state.fieldValue.length === 1) {
            if (this.state.fieldValue.charAt(0) === '-' && value.slice(1).includes('-')) {
                this.setState({ fieldValue: this.state.fieldValue.slice(1) });
            } else if (value.includes('-')) {
                this.setState({ fieldValue: '-'.concat(this.state.fieldValue) });
            }
        }

        // Otherwise validate
        if (floatRegex.test(value)) this.setState({ fieldValue: value });

        if (value.length > 0) this.setState({ confirmEnabled: true });
        else this.setState({ confirmEnabled: false });
    }

    /**
     * Submits changes or resets
     * @param {string} option [close, confirm]
     */
    handleSubmit(option) {
        if (option === 'close') {
            this.resetState();
        }
        if (option === 'confirm') {
            const { control } = this.props;

            let value;
            switch (control.type) {
                case 'enum_slider':
                    value = this.state.checked;
                    break;
                case 'enum_button':
                    value = this.state.pressed;
                    break;
                case 'number':
                    value = parseFloat(this.state.fieldValue);
                    break;
                default:
                    console.log(`ERROR: invalid control type ${control.type} supplied`);
            }
            switch (control.unitPrefix) {
                case 'k':
                    value = (value * 1000) / control.scaler;
                    break;
                case 'M':
                    value = (value * 1000000) / control.scaler;
                    break;
                default:
            }

            let api_path = `${control.base_uri}/${control.category}`;
            if (control.asset_id !== undefined) api_path += `/${control.asset_id}`;

            updatePropertyValue(api_path, control.api_endpoint, value)
                .then((response) => {
                    if (response.ok) {
                        return response.json();
                    }
                    throw new Error(`${response.statusText} : Cannot updatePropertyValue() for: ${response.url}`);
                })
                .catch((error) => {
                    throw new Error(`InputFieldControl/doUpdatePropertyValue error: ${error}`);
                });

            setTimeout(() => {
                this._isMounted && this.resetState();
            }, 1000);
        }
    }

    render() {
        const { control } = this.props;
        const { confirmEnabled, checked } = this.state;

        return (
            <Grid container spacing={3} alignItems="center">
                <Grid item md={8}>
                    {control.type === 'enum_slider'
                        && <FormControl>
                            <FormGroup row>
                                <FormControlLabel
                                    data-cy={`sliderControl_${this.props.control.id}`}
                                    control={
                                        <Switch
                                            checked={checked}
                                            onChange={() => this.handleSwitch()}
                                            color="primary"
                                        />
                                    }
                                    label={control.name}
                                />
                            </FormGroup>
                        </FormControl>
                    }
                    {control.type === 'enum_button'
                        && <Button
                            color="primary"
                            variant="contained"
                            size="small"
                            disabled={!control.enabled || this.state.pressed}
                            onClick={() => this.handleButton()}
                        >
                            {control.name}
                        </Button>
                    }
                    {control.type === 'number'
                        && <TextField
                            value={this.state.fieldValue}
                            onChange={(event) => this.handleTextField(event)}
                            id={control.id}
                            label={control.displayValue}
                            helperText={control.name}
                            inputRef={this.inputField}
                        />
                    }
                </Grid>
                <Grid item md={4}>
                    <Grid item container direction="row" justifyContent="flex-end" >
                        <Grid item>
                            <IconButton
                                color="secondary"
                                disabled={!confirmEnabled}
                                onClick={() => this.handleSubmit('close')}
                                size="large">
                                <Close />
                            </IconButton>
                        </Grid>
                        <Grid item>
                            <IconButton
                                color="primary"
                                disabled={!confirmEnabled}
                                onClick={() => this.handleSubmit('confirm')}
                                size="large">
                                <Check />
                            </IconButton>
                        </Grid>
                    </Grid>
                </Grid>
            </Grid>
        );
    }
}

export default Control;
