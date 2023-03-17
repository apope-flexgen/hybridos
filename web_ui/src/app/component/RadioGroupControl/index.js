/* eslint-disable camelcase */
/* eslint-disable react/prop-types */
import React, { Fragment } from 'react';
import { withStyles } from 'tss-react/mui';
import Radio from '@mui/material/Radio';
import RadioGroup from '@mui/material/RadioGroup';
import FormControlLabel from '@mui/material/FormControlLabel';
import Grid from '@mui/material/Grid';
import FormControl from '@mui/material/FormControl';
import Check from '@mui/icons-material/Check';
import Close from '@mui/icons-material/Close';
import IconButton from '@mui/material/IconButton';
import FormLabel from '@mui/material/FormLabel';
import { STYLES_FEATURES } from '../../styles';
import { updatePropertyValue } from '../../../AppConfig';

/**
 * Used for radio select control
 */
class RadioGroupControl extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            value: null,
            initialValue: null,
            displayConfirm: false,
            enabled: null,
            backgroundColor: '#ffffff',
        };
    }

    /**
     * Initialize values
     */
    componentDidMount() {
        const { control } = this.props;
        this.setState({
            value: control.value.toString(),
            initialValue: control.value.toString(),
            enabled: control.enabled,
            backgroundColor: control.enabled ? '#ffffff' : '#eeeeee',
        });
    }

    /**
     * Handle select logic
     * @param {} event
     */
    handleChange = (event) => {
        this.setState({
            value: event.target.value.toString(),
            displayConfirm: true,
        });
    };

    /**
     * Detects changes
     * @param {*} prevProps
     */
    componentDidUpdate(prevProps) {
        if (this.props.control !== prevProps.control) {
            this.setState({
                enabled: this.props.control.enabled,
                backgroundColor: this.props.control.enabled ? '#ffffff' : '#eeeeee',
            });
        }
    }

    /**
     * Confirmation for control to submit
     */
    confirmChanges() {
        const { control } = this.props;
        const { value } = this.state;
        let api_path = `${control.base_uri}/${control.category}`;
        if (control.asset_id !== undefined) {
            api_path += `/${control.asset_id}`;
        }
        updatePropertyValue(api_path, control.api_endpoint, value)
            .then((response) => {
                if (response.ok) {
                    return response.json();
                }
                throw new Error(`${response.statusText}: Cannot updatePropertyValue() for: ${response.url}`);
            })
            .then(() => {
                this.setState({
                    initialValue: value,
                    displayConfirm: false,
                });
            })
            .catch((error) => {
                throw new Error(`RadioGroupControl/confirmChanges error: ${error}`);
            });
    }

    /**
     * Resets changes
     */
    undoChanges() {
        this.setState({
            value: this.state.initialValue,
            displayConfirm: false,
        });
    }

    render() {
        const { value, displayConfirm } = this.state;
        const { control, classes } = this.props;
        const backgroundColor = control.enabled ? '#ffffff' : '#eeeeee';
        return (
            <Fragment>
                <Grid item md={8} style={{ backgroundColor, padding: 5 }}>
                    <FormControl >
                        <FormLabel>{control.name}</FormLabel>
                        <RadioGroup
                            row
                            aria-label={control.id}
                            name={control.name}
                            value={value ? parseInt(value, 10).toString()
                                : parseInt(control.value, 10).toString()}
                            onChange={this.handleChange}
                        >
                            {control.options.map((element) => (
                                <FormControlLabel
                                    key={element.name.toLowerCase()}
                                    value={element.return_value.toString()}
                                    control={<Radio disabled={this.props.disabled} color="default" />}
                                    label={element.name}
                                />
                            ))}
                        </RadioGroup>
                    </FormControl>
                </Grid>
                <Grid item md={4} style={{ backgroundColor, padding: 5 }}>
                    {displayConfirm
                        ? (<Grid item container direction="row" justifyContent="flex-end" >
                            <Grid item>
                                <IconButton
                                    color="secondary"
                                    disabled={this.props.disabled}
                                    onClick={this.undoChanges.bind(this)}
                                    className={classes.button}
                                    size="large">
                                    <Close />
                                </IconButton>
                            </Grid>
                            <Grid item>
                                <IconButton
                                    color="primary"
                                    disabled={this.props.disabled}
                                    onClick={this.confirmChanges.bind(this)}
                                    className={classes.button}
                                    size="large">
                                    <Check />
                                </IconButton>
                            </Grid>
                        </Grid>)
                        : null}
                </Grid>
            </Fragment >
        );
    }
}
export default withStyles(RadioGroupControl, STYLES_FEATURES);
