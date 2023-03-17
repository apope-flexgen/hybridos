/* eslint-disable no-unused-expressions */
/* eslint-disable no-underscore-dangle */
/* eslint-disable react/prop-types */
/* eslint-disable camelcase */
import React, { Fragment } from 'react';
import PropTypes from 'prop-types';
import FormGroup from '@mui/material/FormGroup';
import FormControlLabel from '@mui/material/FormControlLabel';
import Switch from '@mui/material/Switch';
import Grid from '@mui/material/Grid';
import FormControl from '@mui/material/FormControl';
import Check from '@mui/icons-material/Check';
import Close from '@mui/icons-material/Close';
import IconButton from '@mui/material/IconButton';
import { setOrPutDataForURI, updatePropertyValue } from '../../../AppConfig';
import { connect } from 'react-redux';
import { showMultiUseModal, submitMultiUseModal } from '../../../actions';
import { userRole, userUsername } from '../../../AppAuth';
import { Lockout } from '../Lockout';
/**
 * Component for rendering a slider control
 */
class SliderControl extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            isChecked: false,
            enabled: null,
            display_text: '',
            control: null,
            displayConfirm: false,
            initialValue: null,
            backgroundColor: '#ffffff',
            wait: false,
            modalSubmitted: false,
        };
        this._isMounted = false;
    }

    /**
     * Initializes values
     */
    componentDidMount() {
        this._isMounted = true;
        const { control } = this.props;
        let display_text = control.name;
        if (
            control.base_uri === 'components' &&
            control.options !== undefined
        ) {
            display_text = control.options[0].name;
        }

        this.setState({
            isChecked: control.value,
            initialValue: control.value,
            display_text,
            enabled: control.enabled,
            control,
            backgroundColor: control.enabled ? '#ffffff' : '#eeeeee',
        });
    }

    /**
     * Checks for updates?
     * @param {*} prevProps
     */
    componentDidUpdate(prevProps) {
        if (
            this.props.control.value !== prevProps.control.value ||
            this.props.control.enabled !== prevProps.control.enabled ||
            (this.props.control.value !== this.state.isChecked &&
                !this.state.displayConfirm &&
                !this.state.wait)
        ) {
            this.setState({
                isChecked: this.props.control.value,
                control: this.props.control,
                enabled: this.props.control.enabled,
                backgroundColor: this.props.control.enabled
                    ? '#ffffff'
                    : '#eeeeee',
            });
        }

        if (
            this.props.modalSubmitted === 'success' &&
            this.state.control.value !== this.state.isChecked
        ) {
            this.props.dispatch(submitMultiUseModal('none'));
            this.confirmChanges();
        } else if (this.props.modalSubmitted === 'cancel') {
            this.props.dispatch(submitMultiUseModal('none'));
            this.setState({
                isChecked: this.state.initialValue,
                displayConfirm: false,
            });
        }
    }
    /**
     * Unmounts component
     */
    componentWillUnmount() {
        this._isMounted = false;
    }

    /**
     * Handle select logic
     * @param {} event
     */
    handleChange = (event) => {
        this.setState({
            isChecked: event.target.checked,
            displayConfirm: true,
        });
        if (
            this.props.control.id === 'maint_mode' &&
            event.target.checked === true
        ) {
            this.props.dispatch(showMultiUseModal(true, 'maint_modal'));
        }
    };
    /**
     * Confirmation for control to submit
     */
    confirmChanges() {
        const { isChecked } = this.state;
        const { control } = this.props;
        let api_path = `${control.base_uri}/${control.category}`;
        if (control.asset_id !== undefined) {
            api_path += `/${control.asset_id}`;
        }
        // for components, modbus wants to see 1 or 0, the Material-UI
        // component wants to see true or false. This coercion fixes that. DM 110519
        let isCheckedTemp = isChecked;
        if (control.base_uri === 'components') {
            if (this.state.control.options === undefined) {
                // in the case of tracker stow switches
                isCheckedTemp = isChecked === true ? 3 : 0;
            } else {
                isCheckedTemp =
                    isChecked === true
                        ? this.state.control.options[0].return_value
                        : this.state.control.options[1].return_value;
            }
        }
        updatePropertyValue(api_path, control.api_endpoint, isCheckedTemp)
            .then((response) => {
                if (response.ok) {
                    return response.json();
                }
                throw new Error(
                    `${response.statusText} : Cannot updatePropertyValue() for: ${response.url}`
                );
            })
            .then(() => {
                this.setState(
                    {
                        initialValue: isChecked,
                        displayConfirm: false,
                        wait: true,
                    },
                    () =>
                        setTimeout(
                            () =>
                                this._isMounted &&
                                this.setState({ wait: false }),
                            1000
                        )
                );
            })
            .catch((error) => {
                throw new Error(`SliderControl/confirmChanges error: ${error}`);
            });
    }

    /**
     * Resets changes
     */
    undoChanges() {
        this.setState({
            isChecked: this.state.initialValue,
            displayConfirm: false,
        });
    }

    render() {
        const { isChecked, display_text, displayConfirm } = this.state;
        const { classes, control, disabled } = this.props;
        const backgroundColor = this.props.control.enabled
            ? '#ffffff'
            : '#eeeeee';
        return (
            <Fragment>
                <Grid item md={8} style={{ backgroundColor, padding: 5 }}>
                    <FormControl>
                        <FormGroup row>
                            <FormControlLabel
                                data-cy={`sliderControl_${control.id}`}
                                control={
                                    <Switch
                                        disabled={disabled || !control.enabled}
                                        checked={isChecked}
                                        onChange={this.handleChange.bind(this)}
                                        color={
                                            control.id === 'maint_mode'
                                                ? 'warning'
                                                : 'primary'
                                        }
                                    />
                                }
                                label={display_text}
                            />
                        </FormGroup>
                    </FormControl>
                </Grid>
                <Grid item md={4} style={{ backgroundColor, padding: 5 }}>
                    {displayConfirm ? (
                        <Grid
                            item
                            container
                            direction="row"
                            justifyContent="flex-end"
                        >
                            <Grid item>
                                <IconButton
                                    data-cy={`undoChanges_${control.id}`}
                                    disabled={disabled}
                                    color="secondary"
                                    onClick={this.undoChanges.bind(this)}
                                    className={classes.button}
                                    size="large"
                                >
                                    <Close />
                                </IconButton>
                            </Grid>
                            <Grid item>
                                <IconButton
                                    data-cy={`confirmChanges_${control.id}`}
                                    disabled={disabled}
                                    color="primary"
                                    onClick={this.confirmChanges.bind(this)}
                                    className={classes.button}
                                    size="large"
                                >
                                    <Check />
                                </IconButton>
                            </Grid>
                        </Grid>
                    ) : control.id === 'maint_mode' && isChecked ? (
                        <Grid
                            item
                            container
                            direction="row"
                            justifyContent="flex-end"
                        >
                            <Grid item>
                                <Lockout
                                    category={control.category}
                                    asset_id={control.asset_id}
                                    data={control}
                                    index={this.props.index}
                                    setLockedOut={this.props.setLockedOut}
                                />
                            </Grid>
                        </Grid>
                    ) : null}
                </Grid>
            </Fragment>
        );
    }
}
SliderControl.propTypes = {
    classes: PropTypes.object,
};

const mapStateToProps = (state) => {
    return {
        modalSubmitted: state.multiUseModal.submit,
    };
};

export default connect(mapStateToProps)(SliderControl);
