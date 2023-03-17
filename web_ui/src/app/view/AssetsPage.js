/* eslint-disable no-unused-expressions */
/* eslint-disable no-underscore-dangle */
/* eslint-disable camelcase */
/* eslint-disable react/prop-types */
import React from 'react';
import { withStyles } from 'tss-react/mui';
import Box from '@mui/material/Box';
import Grid from '@mui/material/Grid';
import LoadingHOC from '../component/LoadingHOC';
import { isLoading, getDataForURI, formatAssetObjects } from '../../AppConfig';
import { socket_features, socket_assets, socket_site } from '../../AppAuth';

import InfoBox from '../component/GridView/InfoBox';
import CardGrid from '../component/GridView/CardGrid';

import SingleAsset from '../component/SingleAsset';
import ComponentsPage from './ComponentsPage';
import Alert from '../component/Alert';

import { STYLES_GRID_VIEW } from '../styles';
import { consoleLog } from '../component/Layout';

/**
 * Component for assets page
 */
class AssetsPage extends React.Component {
    /**
     * Initialize assets page
     * @param {object} props set up props for api endpoints and sockets
     */
    constructor(props) {
        super(props);
        let setupProps;
        switch (props.type) {
            case 'features':
                setupProps = {
                    socket_listener_uri: '/features',
                    api_endpoint: 'features',
                    socket: socket_features,
                };
                break;
            case 'site':
                setupProps = {
                    socket_listener_uri: '/site',
                    api_endpoint: 'site',
                    socket: socket_site,
                };
                break;
            case 'generators':
                setupProps = {
                    socket_listener_uri: '/assets/generators',
                    api_endpoint: 'assets/generators',
                    socket: socket_assets,
                };
                break;
            case 'ess':
                setupProps = {
                    socket_listener_uri: '/assets/ess',
                    api_endpoint: 'assets/ess',
                    socket: socket_assets,
                };
                break;
            case 'feeders':
                setupProps = {
                    socket_listener_uri: '/assets/feeders',
                    api_endpoint: 'assets/feeders',
                    socket: socket_assets,
                };
                break;
            case 'solar':
                setupProps = {
                    socket_listener_uri: '/assets/solar',
                    api_endpoint: 'assets/solar',
                    socket: socket_assets,
                };
                break;
            case 'bms':
                setupProps = {
                    socket_listener_uri: '/assets/bms',
                    api_endpoint: 'assets/bms',
                    socket: socket_assets,
                };
                break;
            case 'pcs':
                setupProps = {
                    socket_listener_uri: '/assets/pcs',
                    api_endpoint: 'assets/pcs',
                    socket: socket_assets,
                };
                break;
            default:
                consoleLog(
                    `Incorrect summary card type (${props.type}) supplied`
                );
        }
        this.state = {
            asset_objects: [],
        };
        this._isMounted = false;
        this.state = {
            selected: null,
            ...this.state,
            ...setupProps,
            lockedOut: [],
        };

        this.props.setLoading(isLoading || true);

        this.handleClick = this.handleClick.bind(this);
        this.handleClose = this.handleClose.bind(this);

        this.updateFromSingleAsset = this.updateFromSingleAsset.bind(this);
    }

    // Deprecated as of React 17, but still functional
    // TODO update?
    // eslint-disable-next-line react/no-deprecated
    /**
     * Connect socket based on setup props
     */
    UNSAFE_componentWillMount() {
        // socket recieves data after fetchData() returns successfully
        this.state.socket.on(this.state.socket_listener_uri, (data) => {
            this.updateStateData(JSON.parse(data));
        });
    }

    /**
     * Set mounted and fetch data loop
     */
    componentDidMount() {
        this._isMounted = true;
        setTimeout(() => {
            this._isMounted && this.fetchData();
        }, 1000);
    }

    /**
     * Unmount component and turn off socket
     */
    componentWillUnmount() {
        this.state.socket.off(this.state.socket_listener_uri);
        this._isMounted = false;
    }

    /**
     * Update component when new user clicks new asset
     */
    componentDidUpdate(prevProps, prevState, snapshot) {
        if (this.props.type !== prevProps.type) {
            // need to reset socket on change
            this.state.socket.off(this.state.socket_listener_uri);

            let setupProps;
            switch (this.props.type) {
                case 'features':
                    setupProps = {
                        socket_listener_uri: '/features',
                        api_endpoint: 'features',
                        socket: socket_features,
                    };
                    break;
                case 'site':
                    setupProps = {
                        socket_listener_uri: '/site',
                        api_endpoint: 'site',
                        socket: socket_site,
                    };
                    break;
                case 'generators':
                    setupProps = {
                        socket_listener_uri: '/assets/generators',
                        api_endpoint: 'assets/generators',
                        socket: socket_assets,
                    };
                    break;
                case 'ess':
                    setupProps = {
                        socket_listener_uri: '/assets/ess',
                        api_endpoint: 'assets/ess',
                        socket: socket_assets,
                    };
                    break;
                case 'feeders':
                    setupProps = {
                        socket_listener_uri: '/assets/feeders',
                        api_endpoint: 'assets/feeders',
                        socket: socket_assets,
                    };
                    break;
                case 'solar':
                    setupProps = {
                        socket_listener_uri: '/assets/solar',
                        api_endpoint: 'assets/solar',
                        socket: socket_assets,
                    };
                    break;
                case 'bms':
                    setupProps = {
                        socket_listener_uri: '/assets/bms',
                        api_endpoint: 'assets/bms',
                        socket: socket_assets,
                    };
                    break;
                case 'pcs':
                    setupProps = {
                        socket_listener_uri: '/assets/pcs',
                        api_endpoint: 'assets/pcs',
                        socket: socket_assets,
                    };
                    break;
                default:
                    consoleLog(
                        `Incorrect summary card type (${props.type}) supplied`
                    );
            }
            this.props.setLoading(isLoading || true);
            this.setState(
                {
                    selected: null,
                    asset_objects: [],
                    ...setupProps,
                },
                () => {
                    this.state.socket.on(
                        this.state.socket_listener_uri,
                        (data) => {
                            this.updateStateData(JSON.parse(data));
                        }
                    );
                    setTimeout(() => {
                        this.fetchData();
                    }, 1000); // not sure if timeout needed
                }
            );
        }
    }

    /**
     * Updates state with data from sockets
     * @param {object} data data to insert
     */
    updateStateData(data) {
        sessionStorage.setItem(
            'AssetsPageUpdateStateData',
            parseInt(sessionStorage.getItem('AssetsPageUpdateStateData'), 10) +
                1
        );
        const asset_objects = formatAssetObjects(
            data,
            'assets',
            this.props.type
        );
        let asset_summary;
        const asset_components = [];
        Object.keys(asset_objects).forEach((key) => {
            const object = asset_objects[key];
            if (object.id === 'summary') asset_summary = object;
            else asset_components.push(object);
        });
        this.props.setLoading(false);
        this.setState({
            asset_objects,
            asset_summary,
            asset_components,
            selected_object: asset_summary,
        });
    }

    // Not ideal, data should instead be received from here and passed to single asset
    // Takes the updated asset from single asset and inserts it into its position in asset_objects
    // TODO: hybridOS 2.0
    /**
     * Updates state with data from children component sockets
     * @param {object} asset asset to insert/update
     */
    updateFromSingleAsset(asset) {
        // Map for a deep clone
        const updated_asset_objects = this.state.asset_objects.map(
            (asset_object) =>
                asset_object.id === asset.id ? asset : asset_object
        );
        let asset_summary;
        const asset_components = [];
        Object.keys(updated_asset_objects).forEach((key) => {
            const object = updated_asset_objects[key];
            if (object.id === 'summary') asset_summary = object;
            else asset_components.push(object);
        });
        let updated_selected_object;
        for (let i = 0; i < updated_asset_objects.length; i += 1) {
            const object = updated_asset_objects[i];
            if (
                this.state.selected_object &&
                this.state.selected_object.id === object.id
            )
                updated_selected_object = object;
        }
        this.setState({
            asset_objects: updated_asset_objects,
            asset_summary,
            asset_components,
            selected_object: updated_selected_object,
        });
    }

    /**
     * Get data from uri/socket
     */
    fetchData() {
        this.props.setLoading(isLoading || true);
        getDataForURI(this.state.api_endpoint)
            .then((response) => {
                if (response.ok) {
                    return response.json();
                }
                throw new Error(
                    `${response.statusText} : Cannot get ASSETS for: ${response.url}`
                );
            })
            .catch((error) => this.setState({ error }));
    }

    setLockedOut = (lockedOut, item) => {
        let updated = this.state.lockedOut;
        updated[item] = lockedOut;
        this.setState({
            lockedOut: updated,
        });
    };

    /**
     * [GRID] Handle click in grid view to update parent
     * @param {*} component which child sent the click event
     */
    handleClick(component) {
        this.setState({
            selected_object:
                this.state.selected_object.id === component.id
                    ? this.state.asset_summary
                    : component,
        });
    }

    /**
     * [GRID] Handles closing of selection in grid view
     */
    handleClose() {
        this.setState({
            selected_object: this.state.asset_summary,
        });
    }

    render() {
        const { classes, type, pageType } = this.props;
        const {
            asset_components,
            selected_object,
            // eslint-disable-next-line no-shadow
            isLoading,
            asset_summary,
            asset_objects,
        } = this.state;

        return (
            <React.Fragment>
                {!isLoading && pageType === 'grid' && (
                    <Box
                        style={{
                            display: 'flex',
                            flexDirection: 'row',
                            height: '84vh',
                        }}
                    >
                        <InfoBox
                            selected_object={selected_object}
                            classes={classes}
                            parentCloseHandler={this.handleClose}
                            asset_summary={asset_summary}
                            updateAssetPage={this.updateFromSingleAsset}
                            assetType={type}
                        />
                        <CardGrid
                            asset_components={asset_components}
                            parentClickHandler={this.handleClick}
                            classes={classes}
                            updateAssetPage={this.updateFromSingleAsset}
                            assetType={type}
                            selected_object={selected_object}
                        />
                    </Box>
                )}
                {!isLoading && pageType === 'linear' && (
                    <React.Fragment>
                        {asset_objects.map((asset, index) => (
                            <Alert
                                key={`${asset.name}_${index}`}
                                asset={asset}
                                id={asset.id}
                                classes={classes}
                            />
                        ))}
                        {asset_objects[0] &&
                        asset_objects[0].uri.includes('assets/') ? (
                            ''
                        ) : (
                            <ComponentsPage asset_objects={asset_objects} />
                        )}
                        <Grid data-cy="assetsPage_loaded" container>
                            {asset_objects.map((asset, index) => {
                                if (
                                    asset.status.length !== 0 &&
                                    asset.id !== 'summary'
                                ) {
                                    return (
                                        <SingleAsset
                                            lockedOut={this.state.lockedOut}
                                            setLockedOut={this.setLockedOut}
                                            index={index}
                                            key={asset.id}
                                            asset={asset}
                                            assetType={this.props.type}
                                            updateAssetPage={
                                                this.updateFromSingleAsset
                                            }
                                        />
                                    );
                                }
                                return null;
                            })}
                        </Grid>
                    </React.Fragment>
                )}
            </React.Fragment>
        );
    }
}

export default withStyles(LoadingHOC(AssetsPage), STYLES_GRID_VIEW);
