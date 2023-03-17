/* eslint-disable react/prop-types */
/* eslint-disable camelcase */
import React from 'react';
import Box from '@mui/material/Box';
import Tab from '@mui/material/Tab';
import Tabs from '@mui/material/Tabs';
import AppBar from '@mui/material/AppBar';
import Badge from '@mui/material/Badge';
import IconButton from '@mui/material/IconButton';

import AssignmentOutlinedIcon from '@mui/icons-material/AssignmentOutlined';
import WarningIcon from '@mui/icons-material/Warning';
import SettingsIcon from '@mui/icons-material/Settings';
import ArrowBackIcon from '@mui/icons-material/ArrowBack';

import {
    formatSingleAsset,
} from '../../../AppConfig';
import {
    socket_assets,
} from '../../../AppAuth';
import { consoleLog } from '../Layout';

import GridTab from './GridTab';

let enableTestModeValue = false;

function TabPanel(props) {
    const {
        children,
        value,
        index,
    } = props;

    return (
        <div
            hidden={value !== index}
        >
            {value === index && (
                <Box p={3} display="flex" alignItems="center" flexDirection="column">
                    {children}
                </Box>
            )}
        </div>
    );
}

class InfoBox extends React.Component {
    constructor() {
        super();

        this.state = {
            value: 0,
            asset_summary: {},
            base_uri: 'assets',
            socket_listener_uri: '',
            assetType: '',
        };
    }

    UNSAFE_componentWillMount() {
        const { asset_summary, assetType } = this.props;
        this.setState({
            asset_summary,
            socket_listener_uri: `/${asset_summary.uri}`,
            assetType,
        });

        socket_assets.on(`/${asset_summary.uri}`,
            (data) => this.updateStateData(JSON.parse(data)));
    }

    updateStateData(data) {
        sessionStorage.setItem('SingleAssetUpdateStateData', parseInt(sessionStorage.getItem('SingleAssetUpdateStateData'), 10) + 1);
        // eslint-disable-next-line no-param-reassign
        data.id = this.state.asset_summary.id;
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
                    this.setState({ asset_summary: asset });
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
        const { value } = this.state;
        const { selected_object, classes, parentCloseHandler } = this.props;
        let name;
        if (selected_object) name = selected_object.name;

        let displayAlarmBadge = false;
        if (selected_object) {
            displayAlarmBadge = selected_object.alarms.some(
                (alarm) => alarm.value > 0,
            );
        }

        return (
            <Box style={{
                border: '1px solid #bbb',
                display: 'flex',
                flexDirection: 'column',
                position: 'relative',
                height: '100%',
                backgroundColor: 'white',
                'box-shadow': '1px 2px 3px rgba(0,0,0,0.2)',
            }}>
                { this.props.selected_object && this.props.selected_object.id !== 'summary'
                    && <IconButton
                    onClick={() => parentCloseHandler()}
                    style={{
                        width: '16px', height: '16px', position: 'absolute', top: 6, left: 6,
                    }}
                    size="large">
                        <ArrowBackIcon color='primary' style={{ fontSize: 34 }} />
                    </IconButton>
                }
                <Box style={{ height: '100%', overflow: 'auto' }}>
                    <TabPanel value={value} index={0} style={{ flex: 1 }}>
                        <GridTab
                            component={name}
                            tab='status'
                            data={selected_object}
                        />
                    </TabPanel>
                    <TabPanel value={value} index={1} style={{ flex: 1 }}>
                        <GridTab
                            component={name}
                            tab='alarms'
                            data={selected_object}
                        />
                    </TabPanel>
                    <TabPanel value={value} index={2} style={{ flex: 1 }}>
                        <GridTab
                            component={name}
                            tab='controls'
                            data={selected_object}
                        />
                    </TabPanel>
                </Box>
                <AppBar position="static" style={{ backgroundColor: '#eceff1' }}>
                    <Tabs
                        value={value}
                        onChange={(event, newValue) => { this.setState({ value: newValue }); }}
                        classes={{ indicator: classes.indicator }}
                        style={{ color: 'black' }}
                        indicatorColor='primary'
                    >
                        <Tab icon={<AssignmentOutlinedIcon />} />
                        <Tab icon={<Badge color="secondary" variant="dot" invisible={!displayAlarmBadge}><WarningIcon /></Badge>} />
                        <Tab icon={<SettingsIcon />} />
                    </Tabs>
                </AppBar>
            </Box>
        );
    }
}

export default InfoBox;