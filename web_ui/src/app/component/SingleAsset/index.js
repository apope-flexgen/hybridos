/* eslint-disable react/prop-types */
/* eslint-disable camelcase */
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
import StatusTable from '../StatusTable';
import RadioGroupControl from '../RadioGroupControl';
import InputFieldControl from '../InputFieldControl';
import SliderControl from '../SliderControl';
import ButtonControl from '../ButtonControl';
import { STYLES_FEATURES } from '../../styles';
import { formatSingleAsset, EVENTS_PAGE_PATH } from '../../../AppConfig';
import { socket_assets, userRole } from '../../../AppAuth';
import { consoleLog } from '../Layout';
import { getDataForURI } from '../../../AppConfig';

let enableTestModeValue = false;

class SingleAsset extends React.PureComponent {
    constructor(props) {
        super(props);
        this.state = {
            asset: {},
            base_uri: 'assets',
            socket_listener_uri: '',
            assetType: '',
        };
    }

    // eslint-disable-next-line class-methods-use-this
    UNSAFE_componentWillMount() {
        let { asset, assetType } = this.props;
        asset = this.filterControls(asset);
        this.setState({
            asset,
            socket_listener_uri: `/${asset.uri}`,
            assetType,
        });
        socket_assets.on(`/${asset.uri}`, (data) =>
            this.updateStateData(JSON.parse(data))
        );
    }

    componentWillUnmount() {
        socket_assets.off(this.state.socket_listener_uri);
    }

    updateStateData(data) {
        sessionStorage.setItem(
            'SingleAssetUpdateStateData',
            parseInt(sessionStorage.getItem('SingleAssetUpdateStateData'), 10) +
                1
        );
        // eslint-disable-next-line no-param-reassign
        data.id = this.state.asset.id;
        if (data.enable_test_mode) {
            enableTestModeValue = data.enable_test_mode.value;
            consoleLog('Enable Test Mode:', enableTestModeValue);
        }
        if (
            (enableTestModeValue && data.enable_test_mode) ||
            !enableTestModeValue
        ) {
            consoleLog(
                'updateStateData in Single Asset. Enable Test Mode:',
                enableTestModeValue
            );
            if (this.state[data.id] !== JSON.stringify(data)) {
                this.setState({ [data.id]: JSON.stringify(data) }, () => {
                    console.log(data);
                    let asset = formatSingleAsset(
                        data,
                        this.state.base_uri,
                        this.state.assetType
                    );
                    asset = this.filterControls(asset);
                    this.setState({ asset });
                    // Not ideal. Sends the asset to AssetsPage.js
                    // Instead, AssetsPage.js should pass data as props downward
                    // This is a sockets handling issues and related to backend work
                    // TODO: hybridOS 2.0
                    this.props.updateAssetPage(asset);
                });
            }
        }
    }

    // filters out lock_mode control as it is controlled through a seperate button
    filterControls(asset) {
        asset.controls = asset.controls.filter((control) => {
            return control.id !== 'lock_mode';
        });
        return asset;
    }

    getStateFromChild = (focusedFieldID) => {
        this.setState({
            focusedFieldID,
        });
    };

    render() {
        sessionStorage.setItem(
            'SingleAssetRender',
            parseInt(sessionStorage.getItem('SingleAssetRender'), 10) + 1
        );
        const { asset } = this.state;
        const { classes, index, lockedOut } = this.props;
        return (
            <Grid key={asset.id} item md={12}>
                <Card className={classes.card}>
                    <CardHeader
                        title={
                            enableTestModeValue
                                ? `${asset.name} ->>>>>>>>>> TEST MODE ENABLED <<<<<<<<<<-`
                                : asset.name
                        }
                        action={
                            <Fragment>
                                {asset.faults !== undefined &&
                                    asset.faults.length > 0 &&
                                    asset.faults[0].value > 0 && (
                                        <IconButton
                                            component={Link}
                                            to={EVENTS_PAGE_PATH}
                                            size="large"
                                        >
                                            <Error style={{ color: 'red' }} />
                                        </IconButton>
                                    )}
                                {asset.alarms !== undefined &&
                                    asset.alarms.length > 0 &&
                                    asset.alarms[0].value > 0 && (
                                        <IconButton
                                            component={Link}
                                            to={EVENTS_PAGE_PATH}
                                            size="large"
                                        >
                                            <Warning
                                                style={{ color: 'gold' }}
                                            />
                                        </IconButton>
                                    )}
                            </Fragment>
                        }
                    />
                    <CardContent>
                        <Grid container>
                            <Grid item md={6} lg={6}>
                                <Typography
                                    variant="subtitle1"
                                    align="left"
                                    style={{
                                        borderBottom: '2px solid #ccc',
                                        paddingBottom: 8,
                                    }}
                                >
                                    {' '}
                                    Status
                                </Typography>
                                <StatusTable
                                    status={asset.status}
                                    classes={classes}
                                />
                            </Grid>
                            <Grid item md={1} lg={1}></Grid>
                            <Grid item md={5} lg={5}>
                                {asset.controls.length > 0 && (
                                    <Fragment>
                                        <Typography
                                            variant="subtitle1"
                                            align="left"
                                            style={{
                                                marginBottom: 25,
                                                borderBottom: '2px solid #ccc',
                                                paddingBottom: 8,
                                            }}
                                        >
                                            Controls
                                        </Typography>
                                        {asset.controls.map((control) => (
                                            <Grid
                                                item
                                                key={control.id}
                                                container
                                                md={12}
                                                style={{
                                                    marginTop: 5,
                                                    marginBottom: 10,
                                                }}
                                            >
                                                {control.type === 'enum' && (
                                                    <RadioGroupControl
                                                        disabled={
                                                            userRole ===
                                                                'observer' ||
                                                            lockedOut[index]
                                                        }
                                                        control={control}
                                                        classes={classes}
                                                    />
                                                )}
                                                {control.type ===
                                                    'enum_slider' && (
                                                    <SliderControl
                                                        disabled={
                                                            userRole ===
                                                                'observer' ||
                                                            (control.id ===
                                                            'maint_mode'
                                                                ? false
                                                                : lockedOut[
                                                                      index
                                                                  ])
                                                        }
                                                        control={control}
                                                        classes={classes}
                                                        index={this.props.index}
                                                        setLockedOut={
                                                            this.props
                                                                .setLockedOut
                                                        }
                                                    />
                                                )}
                                                {control.type ===
                                                    'enum_button' && (
                                                    <ButtonControl
                                                        disabled={
                                                            userRole ===
                                                                'observer' ||
                                                            lockedOut[index]
                                                        }
                                                        control={control}
                                                        classes={classes}
                                                    />
                                                )}
                                                {control.type === 'number' && (
                                                    <InputFieldControl
                                                        disabled={
                                                            userRole ===
                                                                'observer' ||
                                                            lockedOut[index]
                                                        }
                                                        value={control.value}
                                                        initialValue={
                                                            control.value
                                                        }
                                                        key={`${asset.id}_${control.id}`}
                                                        id={`${asset.id}_${control.id}`}
                                                        control={control}
                                                        classes={classes}
                                                        getStateFromChild={
                                                            this
                                                                .getStateFromChild
                                                        }
                                                        focusedFieldID={
                                                            this.state
                                                                .focusedFieldID
                                                        }
                                                    />
                                                )}
                                            </Grid>
                                        ))}
                                    </Fragment>
                                )}
                            </Grid>
                        </Grid>
                    </CardContent>
                </Card>
            </Grid>
        );
    }
}
export default withStyles(SingleAsset, STYLES_FEATURES);
