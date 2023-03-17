/* eslint-disable camelcase */
/* eslint-disable react/prop-types */
import React, { Fragment } from 'react';
import PropTypes from 'prop-types';
import Button from '@mui/material/Button';
import Grid from '@mui/material/Grid';
import Check from '@mui/icons-material/Check';
import Close from '@mui/icons-material/Close';
import IconButton from '@mui/material/IconButton';
import { updatePropertyValue } from '../../../AppConfig';

/**
 * Component for rendering control button
 */
class ButtonControl extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            control: null,
            enabled: false,
            isPressed: false,
            displayConfirm: false,
            appearDisabled: false,
            backgroundColor: '#ffffff',
        };
    }

    // eslint-disable-next-line class-methods-use-this
    /**
     * Set initialization on mount
     */
    UNSAFE_componentWillMount() {
        const { control } = this.props;
        this.setState({
            control,
            enabled: control.enabled,
            backgroundColor: control.enabled ? '#ffffff' : '#eeeeee',
        });
    }

    /**
     * Update control on select
     * @param {*} prevProps
     */
    componentDidUpdate(prevProps) {
        if (this.props.control !== prevProps.control) {
            this.setState({
                control: this.props.control,
                enabled: this.props.control.enabled,
                backgroundColor: this.props.control.enabled ? '#ffffff' : '#eeeeee',
            });
        }
    }

    /**
     * Handle control change
     */
    handleChange() {
        this.setState({
            isPressed: true,
            displayConfirm: true,
            appearDisabled: true,
        });
    }

    /**
     * Sets up confirmation for control change
     */
    confirmChanges() {
        const { control } = this.state;
        const return_value = control.options[0].return_value.toString();
        let api_path = `${control.base_uri}/${control.category}`;
        if (control.asset_id !== undefined) {
            api_path += `/${control.asset_id}`;
        }
        updatePropertyValue(api_path, control.api_endpoint, return_value)
            .then((response) => {
                if (response.ok) {
                    return response.json();
                }
                throw new Error(`${response.statusText} : Cannot updatePropertyValue() for: ${response.url}`);
            })
            .then(() => {
                this.setState({
                    isPressed: false,
                    displayConfirm: false,
                });
            })
            .catch((error) => {
                throw new Error(`ButtonControl/confirmChanges error: ${error}`);
            });
    }

    /**
     * Undo change
     */
    undoChanges() {
        this.setState({
            isPressed: false,
            displayConfirm: false,
        });
    }

    /**
     * Deprecated, kept for posterity?
     */
    buttonAppearDisabled() {
        // stopped using this in favor of a simple `!this.props.control.enabled`
        // on 3/26/21. Leaving it around in case it breaks something that I haven't
        // found yet. -DM
        let appear_disabled;
        const { control } = this.props;
        if (control.enabled === true && this.state.isPressed === false) {
            appear_disabled = false;
        }
        if ((control.enabled === true && this.state.isPressed === true)
            || control.enabled === false) {
            appear_disabled = true;
        }
        return appear_disabled;
    }

    render() {
        const { control, displayConfirm } = this.state;
        const backgroundColor = control.enabled ? '#ffffff' : '#eeeeee';
        const { classes } = this.props;
        const color = !this.props.control.enabled ? '#3d3d3d' : '#eeeeee';
        // const color = this.buttonAppearDisabled() ? '#3d3d3d' : '#eeeeee';
        // #3d3d3d passes https://webaim.org/resources/contrastchecker/
        return (
            <Fragment>
                <Grid item md={6} style={{ backgroundColor }}>
                    <Button
                        data-cy={`buttonControl_${this.props.control.id}`}
                        style={{ margin: 8, color: 'black' }}
                        disabled={!this.props.control.enabled || this.props.disabled}
                        // disabled={this.buttonAppearDisabled()}
                        variant="contained"
                        color="primary" // this is the bg color of the button
                        onClick={this.handleChange.bind(this)}
                    >
                        {control.name}
                    </Button>
                </Grid>
                <Grid item md={6} style={{ backgroundColor }}>
                    {displayConfirm
                        ? (<Grid item container direction="row" justifyContent="flex-end" >
                            <Grid item>
                                <IconButton
                                    data-cy={`undoChanges_${this.props.control.id}`}
                                    disabled={this.props.disabled}
                                    color="secondary"
                                    onClick={this.undoChanges.bind(this)}
                                    className={classes.button}
                                    size="large">
                                    <Close />
                                </IconButton>
                            </Grid>
                            <Grid item>
                                <IconButton
                                    data-cy={`confirmChanges_${this.props.control.id}`}
                                    disabled={this.props.disabled}
                                    color="primary"
                                    onClick={this.confirmChanges.bind(this)}
                                    className={classes.button}
                                    size="large">
                                    <Check />
                                </IconButton>
                            </Grid>
                        </Grid>)
                        : null}
                </Grid>
            </Fragment>
        );
    }
}
ButtonControl.propTypes = {
    classes: PropTypes.object.isRequired,
};
export default ButtonControl;
