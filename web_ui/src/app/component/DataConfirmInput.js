import React from 'react';
import { connect } from 'react-redux';

import SocketWrapper from './SocketWrapper';

import { showMultiUseModal, submitMultiUseModal } from '../../actions';
import { SwitchInput, TextInput, ButtonInput, NumberInput } from './Inputs';

import SwitchButtons from './SwitchButtons';

import {
    updatePropertyValue,
    setOrPutDataForURI,
    getDataForURI,
} from '../../AppConfig';

import Grid from '@mui/material/Grid';

import LockIcon from '@mui/icons-material/Lock';

import LockOpenIcon from '@mui/icons-material/LockOpen';
import IconButton from '@mui/material/IconButton';
import { userRole, userUsername } from '../../AppAuth';

import Confirm from './Confirm';
import { Typography, Tooltip } from '@mui/material';
import { setPopupAlert } from '../../actions';
import { Lockout } from './Lockout';

/*
    Props
        baseURI: string
        sourceURI: string
        URI: string
        inputType: string
*/

class DataConfirmInput extends React.Component {
    constructor() {
        super();

        this.state = {
            value: null,
            newValue: null,
            switchOn: false,
            switchOff: false,
            enabled: false,
            isLocked: false,
            displayLock: null,
            lockingUser: '',
        };
    }

    componentDidMount() {
        this.setState({
            displayLock: this.props.URI === '/maint_mode' && this.props.selectedItem !== 'allcontrols' && this.props.selectedItem !== 'summary'
        });
    }

    componentDidUpdate(prevProps, prevState) {
        const {
            disabled,
            inputType,
            data,
            scalar,
            baseURI,
            URI,
            numberOfItems,
            selectedItem,
        } = this.props;
        if (
            disabled &&
            (this.state.value !== '' ||
                this.state.switchOn !== '' ||
                this.state.switchOff !== '')
        ) {
            this.setState({
                value: '',
                newValue: '',
                switchOn: '',
                switchOff: '',
            });
        }

        if (
            inputType === 'switch' &&
            prevProps?.data?.value &&
            data?.value &&
            prevProps.data.value !== data.value
        ) {
            this.setState({
                value: data.value,
                newValue: data.value,
            });
        }

        if (prevProps.baseURI !== baseURI) {
            let value = null;
            let newValue = null;
            switch (inputType) {
                case 'button':
                    value = false;
                    newValue = false;
                    break;
                case 'switch':
                case 'text':
                case 'number':
                    value = null;
                    newValue = null;
                    break;
            }

            this.setState({
                value,
                newValue,
                enabled: false,
            });
        }

        if (prevState.value === null && data !== null && data !== undefined) {
            switch (inputType) {
                case 'switch':
                case 'text':
                    this.setState({
                        value: data.value,
                        newValue: data.value,
                        enabled: true,
                    });
                    break;
                case 'number':
                    this.setState({
                        value:
                            (typeof data === 'object' ? data.value : data) /
                            (scalar ? scalar : 1),
                        newValue:
                            (typeof data === 'object' ? data.value : data) /
                            (scalar ? scalar : 1),
                        enabled: true,
                    });
                    break;
                case 'button':
                    this.setState({
                        value: false,
                        newValue: false,
                        enabled: true,
                    });
                    break;
            }
        }

        if (this.props.modalSubmitted === 'cancel') {
            this.props.dispatch(submitMultiUseModal('none'));
            if (this.props.selectedItem === 'allcontrols')
                this.setState({ switchOn: false, switchOff: false });
            else this.setState({ value: false, newValue: false });
        } else if (
            this.props.modalSubmitted === 'success' &&
            ((this.state.value == false && this.state.newValue == true) ||
                this.state.switchOn == true)
        ) {
            this.props.dispatch(submitMultiUseModal('none'));
            this.handleConfirm(null, 'confirm');
        }
    }
    // sendEvents(...args){

    //     let body = {};
    //     args[0].forEach(a=>{
    //         const key = Object.keys(a)[0];
    //         body[key] = a[key];
    //     })
    //     body.created = Date.now();
    //     body.username = userUsername;
    //     body.userrole = userRole;
    //     body.modified_field = "maintenance_mode";
    //     body.modified_value = true;
    //     let bodyString = JSON.stringify(body);
    //     const uri = `dbi/audit/audit_log_${Date.now()}`
    //     setOrPutDataForURI(uri, bodyString, 'POST')
    //         .catch(e=>{
    //             console.log(e);
    //         })
    // }
    handleChange = (event) => {
        switch (this.props.inputType) {
            case 'switch':
                this.setState({ newValue: event.target.checked });
                if (
                    this.props.URI === '/maint_mode' &&
                    this.state.value === false &&
                    this.state.newValue === false
                ) {
                    this.props.dispatch(showMultiUseModal(true, 'maint_modal'));
                }
                break;
            case 'button':
                this.setState({ newValue: true });
                break;
            case 'text':
                this.setState({ newValue: event.target.value });
                break;
            case 'number':
                this.setState({
                    newValue: event.target.value.match(/^-?\d*\.?\d*$/, '')
                        ? event.target.value
                        : this.state.newValue,
                });
                break;
        }
    };
    handleSwitchButtons = (type) => {
        if (type === 'on') {
            this.setState({
                newValue: true,
                switchOn: !this.state.switchOn,
                switchOff: false,
            });
        } else if (type === 'off') {
            this.setState({
                newValue: false,
                switchOn: false,
                switchOff: !this.state.switchOff,
            });
        }
    };

    handleConfirm = (event, type) => {
        if (type === 'confirm') {
            if (this.props.selectedItem === 'allcontrols') {
                const field = this.props.URI.substring(1);
                let { newValue } = this.state;

                const temp = `${this.props.sourceURI}${this.props.baseURI}`;
                const testURI = temp.substring(1, temp.length);

                let numItems = parseInt(this.props.numberOfItems) + 1;
                const itemURIs = [];

                for (let i = 1; i < numItems; i++) {
                    const temp = testURI.concat(i);
                    itemURIs.push(temp);
                }

                itemURIs.forEach((element) =>
                    updatePropertyValue(element, field, newValue)
                        .then((response) => {
                            if (response.ok) {
                                if (this.props.inputType === 'button')
                                    this.setState({
                                        value: false,
                                        newValue: false,
                                    });
                                else
                                    this.setState({
                                        value: this.state.newValue,
                                        switchOn: false,
                                        switchOff: false,
                                    });
                                return response.json();
                            }

                            throw new Error(
                                `${response.statusText} : Cannot updatePropertyValue() for: ${response.url}`
                            );
                        })
                        .catch((error) => {
                            throw new Error(
                                `ConfirmInput: updatePropertyValue error: ${error}`
                            );
                        })
                );
            } else {
                const fullURI =
                    `${this.props.sourceURI}${this.props.baseURI}`.substring(1);
                const field = this.props.URI.substring(1);
                let { newValue } = this.state;
                if (this.props.inputType === 'number') {
                    // TODO: try parseInt() if it doesn't work
                    newValue =
                        parseFloat(newValue) *
                        (this.props.scalar ? this.props.scalar : 1);
                }
                updatePropertyValue(fullURI, field, newValue)
                    .then((response) => {
                        if (response.ok) {
                            if (this.props.inputType === 'button')
                                this.setState({
                                    value: false,
                                    newValue: false,
                                });
                            else this.setState({ value: this.state.newValue });
                            return response.json();
                        }
                        throw new Error(
                            `${response.statusText} : Cannot updatePropertyValue() for: ${response.url}`
                        );
                    })
                    .catch((error) => {
                        throw new Error(
                            `ConfirmInput: updatePropertyValue error: ${error}`
                        );
                    });
            }
        }
        if (type === 'cancel')
            this.setState({
                newValue: this.state.value,
                switchOn: false,
                switchOff: false,
            });
    };

    render() {
        const { newValue, switchOn, switchOff, isLocked, displayLock } =
            this.state;
        const { name, disabled, URI, selectedItem } = this.props;
        const { handleChange, handleConfirm } = this;

        const lockedOut = URI !== '/maint_mode' && selectedItem !== 'allcontrols' && selectedItem !== 'summary'
            ? this.props.lockedOut[selectedItem] 
            : false

        const enabled =
            (this.props.data !== null &&
                this.props.data !== undefined &&
                this.props.data.enabled &&
                !isLocked) ||
            selectedItem === 'allcontrols';
        return (
            <div
                style={{
                    display: 'flex',
                    justifyContent: 'space-between',
                    alignItems: 'center',
                    width: '100%',
                    marginBottom: '4px',
                }}
            >
                {this.props.inputType === 'switch' &&
                    (selectedItem === 'allcontrols' ? (
                        <SwitchButtons
                            name={name}
                            disabled={disabled || !enabled || lockedOut}
                            switchOn={this.state.switchOn}
                            switchOff={this.state.switchOff}
                            URI={URI}
                            handleSwitchButtons={this.handleSwitchButtons}
                        />
                    ) : (
                        <SwitchInput
                            value={newValue}
                            handleChange={handleChange}
                            label={name}
                            URI={URI}
                            disabled={disabled || !enabled || lockedOut}
                            color="secondary"
                        />
                    ))}
                {this.props.inputType === 'button' && (
                    <ButtonInput
                        value={newValue}
                        handleChange={handleChange}
                        label={name}
                        disabled={disabled || !enabled || lockedOut}
                    />
                )}
                {this.props.inputType === 'text' && (
                    <TextInput
                        value={newValue}
                        handleChange={handleChange}
                        label={name}
                        disabled={disabled || !enabled || lockedOut}
                    />
                )}
                {this.props.inputType === 'number' && (
                    <NumberInput
                        value={newValue}
                        handleChange={handleChange}
                        label={name}
                        disabled={
                            disabled || this.props.data === null || lockedOut
                        }
                        endAdornment={this.props.units}
                    />
                )}
                {displayLock && newValue ? (
                    <Lockout
                        category={this.props.baseURI.split('/')[1]}
                        asset_id={this.props.baseURI.split('/')[2]}
                        index={this.props.selectedItem}
                        data={this.props.data}
                        setLockedOut={this.props.setLockedOut}
                    />
                ) : (
                    <Confirm
                        disabled={
                            selectedItem === 'allcontrols'
                                ? !(switchOff || switchOn)
                                : this.state.value == newValue
                        }
                        handleConfirm={handleConfirm}
                    />
                )}
            </div>
        );
    }
}

const mapStateToProps = (state) => {
    return {
        modalSubmitted: state.multiUseModal.submit,
    };
};

export default connect(mapStateToProps)(DataConfirmInput);
