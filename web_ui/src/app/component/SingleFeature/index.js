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
import Warning from '@mui/icons-material/Warning';
import Error from '@mui/icons-material/Error';
import IconButton from '@mui/material/IconButton';
import { Link } from 'react-router-dom';
import StatusTable from '../StatusTable';
import RadioGroupControl from '../RadioGroupControl';
import InputFieldControl from '../InputFieldControl';
import SliderControl from '../SliderControl';
import ButtonControl from '../ButtonControl';
import { STYLES_FEATURES } from '../../styles';
import {
    formatSingleFeatureSiteObject,
    EVENTS_PAGE_PATH,
} from '../../../AppConfig';
import {
    socket_features,
    userRole
} from '../../../AppAuth';

class SingleFeature extends React.PureComponent {
    constructor(props) {
        super(props);
        this.state = {
            data: {},
            base_uri: 'features',
            socket_listener_uri: '',
            category: '',
        };
    }

    // eslint-disable-next-line class-methods-use-this
    UNSAFE_componentWillMount() {
        const { data } = this.props;

        this.setState({
            data,
            socket_listener_uri: `/${data.uri}`,
            category: data.category,
        });
        socket_features.on(`/${data.uri}`,
            // eslint-disable-next-line no-shadow
            (data) => this.updateStateData(JSON.parse(data)));
    }

    componentWillUnmount() {
        socket_features.off(this.state.socket_listener_uri);
    }

    updateStateData(data) {
        sessionStorage.setItem('SingleFeatureUpdateStateData', parseInt(sessionStorage.getItem('SingleFeatureUpdateStateData'), 10) + 1);
        // eslint-disable-next-line no-param-reassign
        data.id = this.state.data.id;
        if (this.state[data.id] !== JSON.stringify(data)) {
            this.setState({ [data.id]: JSON.stringify(data) }, () => {
                const formattedData = formatSingleFeatureSiteObject(data,
                    this.state.base_uri, this.state.category);
                this.setState({ data: formattedData });
            });
        }
    }

    getStateFromChild = (focusedFieldID) => {
        this.setState({
            focusedFieldID,
        });
    };

    render() {
        sessionStorage.setItem('SingleFeatureRender', parseInt(sessionStorage.getItem('SingleFeatureRender'), 10) + 1);
        const { data } = this.state;
        const { classes } = this.props;
        return (
            <Grid key={data.id} item md={12}>
                <Card className={classes.card}>
                    <CardHeader
                        title={data.name}
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
                                {data.controls.length > 0 && <Fragment>
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
                                }
                            </Grid>
                        </Grid>
                    </CardContent>
                </Card>
            </Grid>
        );
    }
}
SingleFeature.propTypes = {
    classes: PropTypes.object,
};
export default withStyles(SingleFeature, STYLES_FEATURES);