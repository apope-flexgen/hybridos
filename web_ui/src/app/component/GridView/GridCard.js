/* eslint-disable react/prop-types */
/* eslint-disable camelcase */
import React from 'react';
import Grid from '@mui/material/Grid';
import Typography from '@mui/material/Typography';
import Card from '@mui/material/Card';
import CardActionArea from '@mui/material/CardActionArea';
import CardContent from '@mui/material/CardContent';
import Divider from '@mui/material/Divider';

import {
    formatSingleAsset,
} from '../../../AppConfig';
import {
    socket_assets,
    siteConfiguration,
} from '../../../AppAuth';
import { consoleLog } from '../Layout';

let enableTestModeValue = false;

class GridCard extends React.PureComponent {
    constructor() {
        super();

        this.state = {
            asset: {},
            base_uri: 'assets',
            socket_listener_uri: '',
            assetType: '',
        };
    }

    // eslint-disable-next-line class-methods-use-this
    UNSAFE_componentWillMount() {
        const { asset, assetType } = this.props;
        this.setState({
            asset,
            socket_listener_uri: `/${asset.uri}`,
            assetType,
        });
        socket_assets.on(`/${asset.uri}`,
            (data) => this.updateStateData(JSON.parse(data)));
    }

    componentWillUnmount() {
        socket_assets.off(this.state.socket_listener_uri);
    }


    updateStateData(data) {
        sessionStorage.setItem('SingleAssetUpdateStateData', parseInt(sessionStorage.getItem('SingleAssetUpdateStateData'), 10) + 1);
        // eslint-disable-next-line no-param-reassign
        data.id = this.state.asset.id;
        if (data.enable_test_mode) {
            enableTestModeValue = data.enable_test_mode.value;
            consoleLog('Enable Test Mode:', enableTestModeValue);
        }
        if ((enableTestModeValue && data.enable_test_mode) || !enableTestModeValue) {
            consoleLog('updateStateData in Single Asset. Enable Test Mode:', enableTestModeValue);
            if (this.state[data.id] !== JSON.stringify(data)) {
                this.setState({ [data.id]: JSON.stringify(data) }, () => {
                    const asset = formatSingleAsset(
                        data,
                        this.state.base_uri,
                        this.state.assetType,
                    );
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

    render() {
        const {
            component,
            parentClickHandler,
            selected_object,
            assetType,
        } = this.props;

        let status_1;
        let status_2;
        if (component.status
            && siteConfiguration.primaryStatus
            && assetType in siteConfiguration.primaryStatus) {
            status_1 = component.status.find(
                (status) => status.id === siteConfiguration.primaryStatus[assetType][0],
            );
            status_2 = component.status.find(
                (status) => status.id === siteConfiguration.primaryStatus[assetType][1],
            );
        }

        const selected = selected_object.id === component.id;

        return (
            <Card
                variant='outlined'
                style={{
                    borderColor: selected && '#3f51b5',
                    'box-shadow': !selected && '2px 2px 3px rgba(0,0,0,0.2)',
                }}
            >
                <CardActionArea>
                    <CardContent
                        onClick={() => parentClickHandler(component)}
                        style={{ padding: '12px', paddingTop: '6px' }}
                    >
                        <Typography style={{ fontWeight: 'bold', fontSize: '24px', marginBottom: '4px' }}>
                            {component.id}
                        </Typography>
                        {status_1
                            && <Grid container spacing={1}>
                                <Grid item xs={8}>
                                    <Typography style={{ fontWeight: 'bold', fontSize: '16px' }}>{status_1.name}</Typography>
                                </Grid>
                                <Grid item xs={2}>
                                    <Typography align='right'>
                                        {typeof status_1.value === 'number'
                                            ? Math.round(status_1.value * 100) / 100
                                            : status_1.value
                                        }
                                    </Typography>
                                </Grid>
                                <Grid item xs={2}>
                                    <Typography style={{ fontWeight: 'bold' }}>{status_1.unit}</Typography>
                                </Grid>
                            </Grid>
                        }
                        {status_2
                            && <React.Fragment>
                                <Divider style={{ marginBottom: '2px' }} />
                                <Grid container spacing={1}>
                                    <Grid item xs={8}>
                                        <Typography style={{ fontWeight: 'bold', fontSize: '16px' }}>{status_2.name}</Typography>
                                    </Grid>
                                    <Grid item xs={2}>
                                        <Typography align='right'>
                                            {typeof status_2.value === 'number'
                                                ? Math.round(status_2.value * 100) / 100
                                                : status_2.value
                                            }
                                        </Typography>
                                    </Grid>
                                    <Grid item xs={2}>
                                        <Typography style={{ fontWeight: 'bold' }}>{status_2.unit}</Typography>
                                    </Grid>
                                </Grid>
                            </React.Fragment>
                        }
                    </CardContent>
                </CardActionArea>
            </Card>
        );
    }
}

export default GridCard;