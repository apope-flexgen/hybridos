import React from 'react';

import SocketWrapper from './SocketWrapper';

import {
    SwitchInput,
    TextInput,
    ButtonInput,
} from './Inputs';

import { updatePropertyValue } from '../../AppConfig';

import Confirm from './Confirm';

/*
    Props
        baseURI: string
        sourceURI: string
        URI: string
        inputType: string
*/

class ConfirmInput extends React.Component {
    constructor() {
        super();

        this.state = {
            value: null,
            newValue: null,
            enabled: false,
        }
    }

    componentDidUpdate(prevProps, prevState) {
        if (prevProps.baseURI !== this.props.baseURI) {
            this.setState({
                value: null,
                newValue: null,
                enabled: false,
            })
        }

        if (prevState.value === null && this.state.value !== null) {
            this.setState({ newValue: this.state.value });
        }
    }

    update = (data) => {
        const field = this.props.URI.substring(1);
        data = JSON.parse(data);
        let value;
        let enabled;
        if (this.props.inputType === 'button') {
            value = false;
        } else {
            // handling naked vs clothed
            value = typeof data[field] === 'object' ? data[field].value : data[field];
        }
        // TODO: How to connect components in metadata/configuration?
        enabled = typeof data[field] === 'object' ? data[field].enabled : data[field];
        this.setState({
            value,
            enabled
        })
    }

    handleChange = (event) => {
        switch (this.props.inputType) {
            case 'switch':
                this.setState({ newValue: event.target.checked });
                break;
            case 'button':
                this.setState({ newValue: true });
                break;
            case 'text':
                this.setState({ newValue: event.target.value });
                break;
            case 'number':
                this.setState({ newValue: event.target.value.match(/^-?\d*\.?\d*$/, '') ? event.target.value : this.state.newValue });
                break;
        }
    }

    handleConfirm = (event, type) => {
        if (type === 'confirm') {
            const fullURI = `${this.props.sourceURI}${this.props.baseURI}`.substring(1);
            const field = this.props.URI.substring(1);
            updatePropertyValue(fullURI, field, this.state.newValue)
                .then((response) => {
                    if (response.ok) {
                        if (this.props.inputType === 'button') this.setState({ newValue: false });
                        return response.json();
                    }
                    throw new Error(`${response.statusText} : Cannot updatePropertyValue() for: ${response.url}`);
                })
                .catch((error) => {
                    throw new Error(`ConfirmInput: updatePropertyValue error: ${error}`);
                });
                
        }
        if (type === 'cancel') this.setState({ newValue: this.state.value });
    }

    render() {
        const { newValue } = this.state;
        const { name, disabled, baseURI, sourceURI } = this.props;
        const { handleChange, handleConfirm, update } = this;

        return (
            <SocketWrapper
                baseURI={baseURI}
                sourceURI={sourceURI}
                update={update}
            >
                <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', width: '100%', marginBottom: '4px' }}>
                    {this.props.inputType === 'switch'
                        && <SwitchInput
                            value={newValue}
                            handleChange={handleChange}
                            label={name}
                            disabled={disabled || !this.state.enabled}
                        />
                    }
                    {this.props.inputType === 'button'
                        && <ButtonInput
                            value={newValue}
                            handleChange={handleChange}
                            label={name}
                            disabled={disabled || !this.state.enabled}
                        />
                    }
                    {this.props.inputType === 'text'
                        && <TextInput
                            value={newValue}
                            handleChange={handleChange}
                            label={name}
                            disabled={disabled || !this.state.enabled}
                        />
                    }
                    {this.props.inputType === 'number'
                        && <TextInput
                            value={newValue}
                            handleChange={handleChange}
                            label={name}
                            disabled={disabled}
                        />
                    }
                    <Confirm
                        disabled={this.state.value == newValue}
                        handleConfirm={handleConfirm}
                    />
                </div>
            </SocketWrapper>
        );
    }
}

export default ConfirmInput;