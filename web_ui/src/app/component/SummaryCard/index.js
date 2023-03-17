/* eslint-disable no-unused-expressions */
/* eslint-disable no-underscore-dangle */
/* eslint-disable react/prop-types */
/* eslint-disable camelcase */
import React, { Fragment } from 'react';
import { withStyles } from 'tss-react/mui';
import Card from '@mui/material/Card';
import CardContent from '@mui/material/CardContent';
import CardHeader from '@mui/material/CardHeader';
import CardActions from '@mui/material/CardActions';
import Button from '@mui/material/Button';
import { Link } from 'react-router-dom';
import Warning from '@mui/icons-material/Warning';
import Error from '@mui/icons-material/Error';
import IconButton from '@mui/material/IconButton';
import StatusTable from '../StatusTable';
import { STYLES_FEATURES } from '../../styles';
import LoadingHOC from '../LoadingHOC';
import {
    isLoading,
    FEATURES_PAGE_PATH,
    SITE_PAGE_PATH,
    GENERATOR_PAGE_PATH,
    ESS_PAGE_PATH,
    FEEDERS_PAGE_PATH,
    SOLAR_PAGE_PATH,
    EVENTS_PAGE_PATH,
    getPropertiesByUIType,
    getDataForURI,
    BMS_PAGE_PATH,
    PCS_PAGE_PATH,
} from '../../../AppConfig';
import {
    socket_features,
    socket_assets,
    socket_site,
    siteConfiguration,
} from '../../../AppAuth';
import { consoleLog } from '../Layout';

let lastAsset;
let enableTestModeValue = false;

/**
 * Component for rendering a summary card
 */
class SummaryCard extends React.PureComponent {
    /**
     * Sets set up props based off props
     * @param {*} props
     */
    constructor(props) {
        super(props);
        let setupProps;
        switch (props.type) {
            case 'features':
                setupProps = {
                    socket_listener_uri: '/features/summary',
                    api_endpoint: 'features/summary',
                    page_path: FEATURES_PAGE_PATH,
                    title: 'Feature Summary',
                    manage_string: 'Manage Features',
                    socket: socket_features,
                };
                break;
            case 'site':
                setupProps = {
                    socket_listener_uri: '/site/summary',
                    api_endpoint: 'site/summary',
                    page_path: SITE_PAGE_PATH,
                    title: 'Site Summary',
                    manage_string: 'Manage Site',
                    socket: socket_site,
                };
                break;
            case 'generators':
                setupProps = {
                    socket_listener_uri: '/assets/generators/summary',
                    api_endpoint: 'assets/generators/summary',
                    page_path: GENERATOR_PAGE_PATH,
                    title: 'Generator Summary',
                    manage_string: 'Manage Generators',
                    socket: socket_assets,
                };
                break;
            case 'ess':
                setupProps = {
                    socket_listener_uri: '/assets/ess/summary',
                    api_endpoint: 'assets/ess/summary',
                    page_path: ESS_PAGE_PATH,
                    title: 'Storage Summary',
                    manage_string: 'Manage Storage',
                    socket: socket_assets,
                };
                break;
            case 'feeders':
                setupProps = {
                    socket_listener_uri: '/assets/feeders/summary',
                    api_endpoint: 'assets/feeders/summary',
                    page_path: FEEDERS_PAGE_PATH,
                    title: 'Feeders Summary',
                    manage_string: 'Manage Feeders',
                    socket: socket_assets,
                };
                break;
            case 'solar':
                setupProps = {
                    socket_listener_uri: '/assets/solar/summary',
                    api_endpoint: 'assets/solar/summary',
                    page_path: SOLAR_PAGE_PATH,
                    title: 'Solar Summary',
                    manage_string: 'Manage Solar',
                    socket: socket_assets,
                };
                break;
            case 'bms':
                setupProps = {
                    socket_listener_uri: '/assets/bms/summary',
                    api_endpoint: 'assets/bms/summary',
                    page_path: BMS_PAGE_PATH,
                    title: 'BMS Summary',
                    manage_string: 'Manage BMS',
                    socket: socket_assets,
                };
                break;
            case 'pcs':
                setupProps = {
                    socket_listener_uri: '/assets/pcs/summary',
                    api_endpoint: 'assets/pcs/summary',
                    page_path: PCS_PAGE_PATH,
                    title: 'PCS Summary',
                    manage_string: 'Manage PCS',
                    socket: socket_assets,
                };
                break;
            default:
                consoleLog(`Incorrect summary card type (${props.type}) supplied`);
        }
        this.state = {
            status: {},
            error: '',
            faults: [],
            alarms: [],
        };
        this.props.setLoading(true)
        this._isMounted = false;
        this.state = { ...this.state, ...setupProps };
    }

    /**
     * Connect sockets and configure metadata
     */
    // eslint-disable-next-line class-methods-use-this
    UNSAFE_componentWillMount() {
        this.state.socket.on(this.state.socket_listener_uri,
            (data) => this.updateStateData(JSON.parse(data)));
        let existingAsset;
        // if (siteConfiguration.metadata) {
        //     const splitEndpoint = this.state.api_endpoint.split('/');
        //     // eslint-disable-next-line default-case
        //     switch (splitEndpoint.length) {
        //         case 1:
        //             existingAsset = siteConfiguration.metadata[splitEndpoint[0]];
        //             break;
        //         case 2:
        //             if (siteConfiguration.metadata[splitEndpoint[0]]) {
        //                 existingAsset = siteConfiguration
        //                     .metadata[splitEndpoint[0]][splitEndpoint[1]];
        //             }
        //             break;
        //         case 3:
        //             if (siteConfiguration.metadata[splitEndpoint[0]][splitEndpoint[1]]) {
        //                 existingAsset = siteConfiguration
        //                     .metadata[splitEndpoint[0]][splitEndpoint[1]][splitEndpoint[2]];
        //             }
        //             break;
        //         case 4:
        //             if (siteConfiguration.metadata[splitEndpoint[0]][splitEndpoint[1]]
        //                 && siteConfiguration
        //                     .metadata[splitEndpoint[0]][splitEndpoint[1]][splitEndpoint[2]]) {
        //                 // eslint-disable-next-line max-len
        //                 existingAsset = siteConfiguration.metadata[splitEndpoint[0]][splitEndpoint[1]][splitEndpoint[2]][splitEndpoint[3]];
        //             }
        //             break;
        //     }
        //     if (existingAsset) {
        //         const status = getPropertiesByUIType(existingAsset, 'status');
        //         Object.keys(status).map((item) => {
        //             const theBaseURI = this.state.api_endpoint.split('/')[0];
        //             status[item].base_uri = theBaseURI;
        //             status[item].category = this.state.api_endpoint.replace(`${theBaseURI}/`, '');
        //             return null;
        //         });
        //         existingAsset.status = status;
        //     }
        // }
        this.setState({ existingAsset });
    }

    /**
     * Start socket polling
     */
    componentDidMount() {
        this._isMounted = true;
        setTimeout(() => {
            this._isMounted && this.fetchData();
        }, 1000);
    }

    /**
     * Disconnect sockets
     */
    componentWillUnmount() {
        this.state.socket.off(this.state.socket_listener_uri);
        this._isMounted = false;
    }

    /**
     * Updates state with socket data and passes it upwards
     * @param {object} data data to insert
     */
    updateStateData(data) {
        const { existingAsset } = this.state;
        if (!Object.keys(data).includes('name') && existingAsset) {
            for (let i = 0; i < Object.keys(data).length; i += 1) {
                const [key, value] = Object.entries(data)[i];
                if (existingAsset.status) {
                    existingAsset.status.forEach((obj) => {
                        if (key === obj.api_endpoint) {
                            // eslint-disable-next-line no-param-reassign
                            obj.value = value;
                        }
                    });
                }
            }
            this.setState({
                status: existingAsset.status,
            });
            this.props.setLoading(false)
            // else we assume it is an "old school" clothed body
        } else {
            if (data.enable_test_mode) {
                enableTestModeValue = data.enable_test_mode.value;
                consoleLog('Enable Test Mode:', enableTestModeValue);
            }
            if ((enableTestModeValue && data.enable_test_mode) || !enableTestModeValue) {
                consoleLog('updateStateData in Single Asset. Enable Test Mode:', enableTestModeValue);
                if (lastAsset !== JSON.stringify(data)) {
                    lastAsset = JSON.stringify(data);
                    sessionStorage.setItem('SummaryCardUpdateStateData', parseInt(sessionStorage.getItem('SummaryCardUpdateStateData'), 10) + 1);
                    const status = getPropertiesByUIType(data, 'status');
                    const faults = getPropertiesByUIType(data, 'fault');
                    const alarms = getPropertiesByUIType(data, 'alarm');
                    Object.keys(status).map((item) => {
                        const theBaseURI = this.state.api_endpoint.split('/')[0];
                        status[item].base_uri = theBaseURI;
                        status[item].category = this.state.api_endpoint.replace(`${theBaseURI}/`, '');
                        return null;
                    });
                    let processedData;
                    if (existingAsset) {
                        if (existingAsset.status && existingAsset.status !== {}) {
                            existingAsset.status.forEach((obj, i) => {
                                if (status) {
                                    const theIndex = status
                                        .findIndex((x) => x.api_endpoint === obj.api_endpoint);
                                    existingAsset.status[i].value = theIndex !== -1
                                        ? status[theIndex].value : 'null';
                                }
                            });
                        }
                        processedData = existingAsset.status;
                    } else {
                        processedData = status;
                    }
                    this.props.setLoading(false)
                    this.setState({
                        status: processedData,
                        faults,
                        alarms,
                    });
                }
            }
        }
    }

    /**
     * Fetches data from socket
     */
    fetchData() {
        this.props.setLoading(isLoading || true)
        getDataForURI(this.state.api_endpoint)
            .then((response) => {
                if (response.ok) {
                    return response.json();
                }
                throw new Error(`${response.statusText} : Cannot get ${this.state.api_endpoint} for: ${response.url}`);
            })
            .catch((error) => this.updateStateData({ error }));
    }

    render() {
        sessionStorage.setItem('SummaryCardRender', parseInt(sessionStorage.getItem('SummaryCardRender'), 10) + 1);
        const {
            // eslint-disable-next-line no-shadow
            status, faults, alarms, isLoading, page_path, title, manage_string,
        } = this.state;
        const { classes } = this.props;
        return <>
            <Card style={{ padding: 10, paddingTop: 5 }}>
                <CardHeader
                    title={enableTestModeValue ? `${title} ->>>>>>>>>> TEST MODE ENABLED <<<<<<<<<<-` : title}
                    action={
                        <Fragment>
                            {(faults !== undefined && faults.length > 0 && faults[0].value > 0)
                                && <IconButton component={Link} to={EVENTS_PAGE_PATH} size="large"><Error style={{ color: 'red' }} /></IconButton>
                            }

                            {(alarms !== undefined && alarms.length > 0 && alarms[0].value > 0)
                                && <IconButton component={Link} to={EVENTS_PAGE_PATH} size="large"><Warning style={{ color: 'gold' }} /></IconButton>
                            }
                        </Fragment>
                    }
                />
                <CardActions>
                    <Button data-cy={`summaryCard_${manage_string.toLowerCase().replace(/ /g, '_')}`} component={Link} to={page_path} color="primary" variant="outlined" fullWidth>{manage_string}</Button>
                </CardActions>
            </Card>
        </>;
    }
}
export default withStyles(LoadingHOC(SummaryCard), STYLES_FEATURES);
