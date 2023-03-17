/* eslint-disable react/no-string-refs */
/* eslint-disable no-unused-expressions */
/* eslint-disable camelcase */
/* eslint-disable react/prop-types */
/* eslint-disable no-underscore-dangle */
import React, { Component, Fragment } from 'react';
import { withStyles } from 'tss-react/mui';
import Grid from '@mui/material/Grid';
import FormControl from '@mui/material/FormControl';
import TextField from '@mui/material/TextField';
import InputAdornment from '@mui/material/InputAdornment';
import classNames from 'classnames';
import Check from '@mui/icons-material/Check';
import Close from '@mui/icons-material/Close';
import IconButton from '@mui/material/IconButton';
import { updatePropertyValue } from '../../../AppConfig';
import Keypad from '../Keypad/index';
import { STYLES_FEATURES } from '../../styles';

/**
 * Renders input field control type
 */
class InputFieldControl extends Component {
    constructor() {
        super();
        this.state = {
            fieldValue: '',
            showKeypad: false,
        };
        this.inputField = React.createRef();

        this.handleTextField = this.handleTextField.bind(this);
        this.handleFieldFocus = this.handleFieldFocus.bind(this);
        this.handleSubmit = this.handleSubmit.bind(this);
    }

    componentDidMount() {
        this._isMounted = true;
    }

    componentWillUnmount() {
        this._isMounted = false;
    }

    /**
     * Updates property value on confirmation
     */
    doUpdatePropertyValue() {
        const { control } = this.props;

        let value = parseFloat(this.state.fieldValue);
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
            this._isMounted && this.setState({
                fieldValue: '',
            });
            this._isMounted && this.inputField.current.blur();
        }, 1000);
    }

    /**
     * Displays element in console on click
     * @param {*} event click event
     */
    handleClick(event) {
        if (document.getElementById('inspector') !== null) {
            console.log(this);
            sessionStorage.setItem('lastReference', this);
        }
    }

    /**
     * Handles text field logic from physical and virtual keyboards
     * @param {*} event typing event
     * @param {true} fromVirtualKeypad true if from virtual keyboard
     */
    handleTextField(event, fromVirtualKeypad) {
        const { value } = event.target;

        if (fromVirtualKeypad) {
            if (document.activeElement !== document.getElementById(`${this.props.id}`)) this.inputField.current.focus();

            const { selectionStart, selectionEnd } = this.inputField.current;
            switch (value) {
                case 'dismiss':
                    this.setState({ showKeypad: false });
                    this.inputField.current.blur();
                    break;
                case 'backspace':
                    if (selectionStart === selectionEnd) {
                        if (selectionStart !== 0) {
                            this.setState({
                                fieldValue:
                                this.state.fieldValue.slice(0, selectionStart - 1)
                                + this.state.fieldValue.slice(selectionStart),
                            }, () => {
                                this.inputField.current.selectionStart = selectionStart - 1;
                                this.inputField.current.selectionEnd = selectionStart - 1;
                            });
                        }
                    } else {
                        this.setState({
                            fieldValue:
                            this.state.fieldValue.slice(0, selectionStart)
                            + this.state.fieldValue.slice(selectionEnd),
                        }, () => {
                            this.inputField.current.selectionStart = selectionStart;
                            this.inputField.current.selectionEnd = selectionStart;
                        });
                    }
                    break;
                case 'sign-change':
                    if (this.state.fieldValue.charAt(0) !== '-') {
                        this.setState({
                            fieldValue: '-'.concat(this.state.fieldValue),
                        }, () => {
                            this.inputField.current.selectionStart = selectionStart + 1;
                            this.inputField.current.selectionEnd = selectionEnd + 1;
                        });
                    } else {
                        this.setState({
                            fieldValue: this.state.fieldValue.slice(1),
                        }, () => {
                            this.inputField.current.selectionStart = selectionStart - 1;
                            this.inputField.current.selectionEnd = selectionEnd - 1;
                        });
                    }
                    break;
                case 'decimal':
                    if (!this.state.fieldValue.includes('.')) {
                        this.setState({
                            fieldValue:
                            `${this.state.fieldValue.slice(0, selectionStart)}.${this.state.fieldValue.slice(selectionEnd)}`,
                        }, () => {
                            this.inputField.current.selectionStart = selectionStart + 1;
                            this.inputField.current.selectionEnd = selectionStart + 1;
                        });
                    }
                    break;
                default:
                    this.setState({
                        fieldValue:
                        this.state.fieldValue.slice(0, selectionStart)
                        + value
                        + this.state.fieldValue.slice(selectionEnd),
                    }, () => {
                        this.inputField.current.selectionStart = selectionStart + 1;
                        this.inputField.current.selectionEnd = selectionStart + 1;
                    });
            }
        } else {
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
        }
    }

    /**
     * Shows virtual keyboard on focus
     */
    handleFieldFocus() {
        this.setState({ showKeypad: true });
    }

    /**
     * Confirmation or cancellation of submit
     * @param {string} type accept or cancel
     */
    handleSubmit(type) {
        this.setState({ showKeypad: false });
        if (type === 'accept') {
            this.doUpdatePropertyValue();
        }
        if (type === 'cancel') {
            this.setState({ fieldValue: '' });
            this.inputField.current.blur();
        }
    }

    render() {
        const {
            control, classes, id,
        } = this.props;
        const backgroundColor = control.enabled ? '#ffffff' : '#eeeeee';

        return (
            <Fragment>
                <Grid item md={8} style={{ backgroundColor, paddingLeft: 5 }} onClick={this.handleClick.bind(`/${control.base_uri}/${control.category}${control.asset_id ? `/${control.asset_id}` : ''} '{"${control.api_endpoint}": ${control.value}}'`)}>
                    <FormControl style={{ marginTop: 8, marginBottom: 8 }}>
                        <TextField
                            value={this.state.fieldValue}
                            onChange={(event) => this.handleTextField(event)}
                            onFocus={this.handleFieldFocus}
                            onKeyPress={(event) => { if (event.key === 'Enter') this.handleSubmit('accept'); }}
                            id={id}
                            className={classNames(classes.margin, classes.textField)}
                            label={control.displayValue}
                            InputLabelProps={{ style: { color: 'black' } }}
                            helperText={control.name}
                            FormHelperTextProps={{ style: { color: 'black' } }}
                            InputProps={{
                                endAdornment: <InputAdornment position='end'>{control.unitPrefix}{control.unit}</InputAdornment>,
                            }}
                            inputRef={this.inputField}
                            disabled={this.props.disabled}
                        />
                    </FormControl>
                </Grid>
                <Grid item md={4} style={{ backgroundColor, padding: 5 }}>
                    <span className='NOT-reset-save' ref='reset_save'>
                        {/* Display only when at least 1 digit inputted */}
                        { /^-?.?\d+/.test(this.state.fieldValue)
                            && <Grid item container direction="row" justifyContent="flex-end" >
                                <Grid item>
                                    <IconButton
                                        color="secondary"
                                        disabled={this.props.disabled}
                                        onClick={() => { this.handleSubmit('cancel'); }}
                                        className={classes.button}
                                        size="large">
                                        <Close />
                                    </IconButton>
                                </Grid>
                                <Grid item>
                                    <IconButton
                                        color="primary"
                                        disabled={this.props.disabled}
                                        onClick={() => { this.handleSubmit('accept'); }}
                                        className={classes.button}
                                        size="large">
                                        <Check />
                                    </IconButton>
                                </Grid>
                            </Grid>
                        }
                    </span>
                </Grid>
                {this.state.showKeypad
                    && <Keypad parentInputHandler={this.handleTextField} />
                }
            </Fragment>
        );
    }
}
export default withStyles(InputFieldControl, STYLES_FEATURES);
