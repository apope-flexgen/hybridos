/* eslint-disable react/prop-types */
/* eslint-disable camelcase */
import React, { Fragment } from 'react';
import PropTypes from 'prop-types';
import { withStyles } from 'tss-react/mui';
import Grid from '@mui/material/Grid';
import Card from '@mui/material/Card';
import CardContent from '@mui/material/CardContent';
import CardHeader from '@mui/material/CardHeader';
import Typography from '@mui/material/Typography';
import { Link } from 'react-router-dom';
import Warning from '@mui/icons-material/Warning';
import Error from '@mui/icons-material/Error';
import IconButton from '@mui/material/IconButton';
import StatusTable from '../StatusTable';
import RadioGroupControl from '../RadioGroupControl';
import SliderControl from '../SliderControl';
import ButtonControl from '../ButtonControl';
import InputFieldControl from '../InputFieldControl';
import { STYLES_FEATURES } from '../../styles';
import {
    formatSingleFeatureSiteObject,
    EVENTS_PAGE_PATH,
} from '../../../AppConfig';
import {
    socket_site,
    userRole
} from '../../../AppAuth';
import { consoleLog } from '../Layout';

let enableTestModeValue = false;

class SingleSite extends React.PureComponent {
    constructor(props) {
        super(props);
        this.state = {
            data: {},
            base_uri: 'site',
            socket_listener_uri: '',
            category: '',
        };
    }

    // eslint-disable-next-line class-methods-use-this
    UNSAFE_componentWillMount() {
        // eslint-disable-next-line react/prop-types
        const { data } = this.props;
        this.setState({
            data,
            socket_listener_uri: `/${data.uri}`,
            category: data.category,
        });
        // eslint-disable-next-line no-shadow
        socket_site.on(`/${data.uri}`, (data) => {
            this.updateStateData(JSON.parse(data));
        });
    }

    componentWillUnmount() {
        socket_site.off(this.state.socket_listener_uri);
    }

    updateStateData(data) {
        sessionStorage.setItem('SingleSiteUpdateStateData', parseInt(sessionStorage.getItem('SingleSiteUpdateStateData'), 10) + 1);
        // eslint-disable-next-line no-param-reassign
        data.id = this.state.data.id;
        if (data.enable_test_mode) {
            enableTestModeValue = data.enable_test_mode.value;
            consoleLog('Enable Test Mode:', enableTestModeValue);
        }
        if ((enableTestModeValue && data.enable_test_mode) || !enableTestModeValue) {
            consoleLog('updateStateData in Single Site. Enable Test Mode:', enableTestModeValue);
            if (this.state[data.id] !== JSON.stringify(data)) {
                this.setState({ [data.id]: JSON.stringify(data) }, () => {
                    const formattedData = formatSingleFeatureSiteObject(data,
                        this.state.base_uri, this.state.category);
                    this.setState({ data: formattedData });
                    // Not ideal. Sends the asset to Site.js
                    // Instead, Site.js should pass data as props downward
                    // This is a sockets handling issues and related to backend work
                    // TODO: hybridOS 2.0
                    this.props.updateSitePage(formattedData);
                });
            }
        }
    }

    getStateFromChild = (focusedFieldID) => {
        this.setState({
            focusedFieldID,
        });
    };

    render() {
        sessionStorage.setItem('SingleSiteRender', parseInt(sessionStorage.getItem('SingleSiteRender'), 10) + 1);
        const { data } = this.state;
        // eslint-disable-next-line react/prop-types
        const { classes } = this.props;
        return (
            <Grid key={data.id} item md={12}>
                <Card className={classes.card}>
                    <CardHeader
                        title={enableTestModeValue ? `${data.name} ->>>>>>>>>> TEST MODE ENABLED <<<<<<<<<<-` : data.name}
                        action={
                            <Fragment>
                                {(data.faults !== undefined && data.faults.length > 0
                                    && data.faults[0].value > 0)
                                    && <IconButton component={Link} to={EVENTS_PAGE_PATH} size="large"><Error style={{ color: 'red' }} /></IconButton>
                                }
                                {(data.alarms !== undefined && data.alarms.length > 0
                                    && data.alarms[0].value > 0)
                                    && <IconButton component={Link} to={EVENTS_PAGE_PATH} size="large"><Warning style={{ color: 'gold' }} /></IconButton>
                                }
                            </Fragment>
                        }
                    />
                    <CardContent>
                        <Grid container>
                            <Grid item md={6} lg={6}>
                                <Typography variant="subtitle1" align="left" style={{ borderBottom: '2px solid #ccc', paddingBottom: 8 }}> Status</Typography>
                                <StatusTable status={data.status} classes={classes} />
                            </Grid>
                            <Grid item md={1} lg={1}></Grid>
                            <Grid item md={5} lg={5}>
                                <Fragment>
                                    <Typography variant="subtitle1" align="left" style={{ marginBottom: 25, borderBottom: '2px solid #ccc', paddingBottom: 8 }}>Controls</Typography>
                                    {data.controls.map((control) => (
                                        <Grid item key={control.id} container md={12}
                                            style={{ marginTop: 5, marginBottom: 10 }}>
                                            {control.type === 'enum' && <RadioGroupControl disabled={userRole === 'observer'} control={control}
                                                classes={classes} />}
                                            {control.type === 'enum_slider' && <SliderControl disabled={userRole === 'observer'} control={control}
                                                classes={classes} />}
                                            {control.type === 'enum_button' && <ButtonControl disabled={userRole === 'observer'} control={control}
                                                classes={classes} />}
                                            {control.type === 'number' && <InputFieldControl
                                                disabled={userRole === 'observer'}
                                                value={control.value}
                                                initialValue={control.value}
                                                key={`${data.id}_${control.id}`}
                                                id={`${data.id}_${control.id}`}
                                                control={control}
                                                classes={classes}
                                                getStateFromChild={this.getStateFromChild}
                                                focusedFieldID={this.state.focusedFieldID}
                                            />}
                                        </Grid>
                                    ))}
                                </Fragment>
                            </Grid>
                        </Grid>
                    </CardContent>
                </Card>
            </Grid>
        );
    }
}
SingleSite.propTypes = {
    classes: PropTypes.object,
    data: PropTypes.object,
};
export default withStyles(SingleSite, STYLES_FEATURES);