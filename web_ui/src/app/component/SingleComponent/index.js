/* eslint-disable react/prop-types */
import React, { Fragment } from 'react';
import { withStyles } from 'tss-react/mui';
import Grid from '@mui/material/Grid';
import Card from '@mui/material/Card';
import CardContent from '@mui/material/CardContent';
import CardHeader from '@mui/material/CardHeader';
import Typography from '@mui/material/Typography';
import Warning from '@mui/icons-material/Warning';
import Error from '@mui/icons-material/Error';
import IconButton from '@mui/material/IconButton';
import { Link } from 'react-router-dom';
import ComponentStatusTable from '../ComponentStatusTable';
import RadioGroupControl from '../RadioGroupControl';
import InputFieldControl from '../InputFieldControl';
import SliderControl from '../SliderControl';
import ButtonControl from '../ButtonControl';
import { STYLES_FEATURES } from '../../styles';
import {
    // SITE_CONFIG,
    getPropertiesByUIType,
    EVENTS_PAGE_PATH,
} from '../../../AppConfig';

/**
 * Renders a single component (non react)
 */
class SingleComponent extends React.Component { // do not set to PureComponent,
    // numbers will not update
    constructor(props) {
        super(props);
        this.state = {
        };
    }

    /**
     * Initializes state? (could probably be done in constructor)
     */
    // eslint-disable-next-line camelcase
    UNSAFE_componentWillMount() {
        const { component, classes } = this.props;
        // getPropertiesByUIType(component, null);
        this.setState({
            component,
            classes,
        });
    }

    /**
     * Used for focusing text inputs
     * @param {*} focusedFieldID ID of text input
     */
    getStateFromChild = (focusedFieldID) => {
        this.setState({
            focusedFieldID,
        });
    };

    render() {
        const { component, classes } = this.state;
        getPropertiesByUIType(component, null);
        return (
            <Grid item md={12}>
                <Card>
                    <CardHeader
                        title={component.name}
                        action={
                            <Fragment>
                                {(component.faults !== undefined && component.faults.length > 0
                                    && component.faults[0].name > '') // TODO: `.value > 0` instead of .name... ??
                                    && <IconButton component={Link} to={EVENTS_PAGE_PATH} size="large"><Error style={{ color: 'red' }} /></IconButton>
                                }
                                {(component.alarms !== undefined && component.alarms.length > 0
                                    && component.alarms[0].name > '')
                                    && <IconButton component={Link} to={EVENTS_PAGE_PATH} size="large"><Warning style={{ color: 'gold' }} /></IconButton>
                                }
                            </Fragment>
                        }
                    />
                    <CardContent>
                        <Grid container>
                            <Grid item md={6} lg={6}>
                                <Typography variant="subtitle1" align="left" style={{ borderBottom: '2px solid #ccc', paddingBottom: 8 }}> Status</Typography>
                                {component.statuses.map((status, i) => <ComponentStatusTable key={i} status={status} componentID={component.controls[0] ? `/${component.controls[0].base_uri}/${component.id}` : `/components/${component.id}`} />)}
                            </Grid>
                            <Grid item md={1} lg={1}></Grid>
                            <Grid item md={5} lg={5}>
                                {component.controls.length > 0 && <Fragment>
                                    <Typography variant="subtitle1" align="left" style={{ marginBottom: 25, borderBottom: '2px solid #ccc', paddingBottom: 8 }}>Controls</Typography>
                                    {component.controls.map((control) => {
                                        // eslint-disable-next-line no-param-reassign
                                        control.enabled = true; // TODO:this is a
                                        // kludge until we figure out how we want
                                        // to enable/disable components
                                        return (
                                            <Grid item key={control.id} container md={12}
                                                style={{ marginTop: 5, marginBottom: 10 }}>
                                                {control.type === 'enum' && <RadioGroupControl control={control} classes={classes} />}
                                                {control.type === 'enum_slider' && <SliderControl control={control} classes={classes} /> /* used for on/off switches */}
                                                {control.type === 'enum_button' && <ButtonControl control={control} classes={classes} />}
                                                {control.type === 'number' && <InputFieldControl
                                                    value={control.value}
                                                    initialValue={control.value}
                                                    key={`${component.id}_${control.id}`}
                                                    id={`${component.id}_${control.id}`}
                                                    control={control}
                                                    classes={classes}
                                                    getStateFromChild={this.getStateFromChild}
                                                    focusedFieldID={this.state.focusedFieldID}
                                                />}
                                            </Grid>
                                        );
                                    })}
                                </Fragment>
                                }
                            </Grid>
                        </Grid>
                    </CardContent>
                </Card>
            </Grid>
        );
    }
}
export default withStyles(SingleComponent, STYLES_FEATURES);
