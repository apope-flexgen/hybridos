/* eslint-disable no-unused-expressions */
/* eslint-disable no-underscore-dangle */
/* eslint-disable camelcase */
import React, { Fragment } from 'react';
import PropTypes from 'prop-types';
import { withStyles } from 'tss-react/mui';
import Grid from '@mui/material/Grid';
import SingleSite from '../component/SingleSite';
import { STYLES_FEATURES } from '../styles';
import LoadingHOC from '../component/LoadingHOC';
import {
    isLoading,
    getDataForURI,
    formatFeatureSiteObjects,
} from '../../AppConfig';
import {
    socket_site,
} from '../../AppAuth';
import Alert from '../component/Alert';

/**
 * Component for rendering site
 */
class Site extends React.PureComponent {
    constructor(props) {
        super(props);
        this.state = {
            formatted_data_objects: [],
            api_endpoint: 'site',
            socket_listener_uri: '/site',
        };
        this.props.setLoading(isLoading || true)

        this._isMounted = false;

        this.updateFromSingleSite = this.updateFromSingleSite.bind(this);
    }

    // we are using React 16.4.1 - before the deprecation referenced below
    // eslint-disable-next-line react/no-deprecated
    /**
     * Connect socket
     */
    UNSAFE_componentWillMount() {
        socket_site.on(this.state.socket_listener_uri, (data) => {
            this.updateStateData(JSON.parse(data));
        });
    }

    /**
     * Start socket fetch loop
     */
    componentDidMount() {
        this._isMounted = true;
        setTimeout(() => {
            this._isMounted && this.fetchData();
        }, 1000);
    }

    /**
     * Unmount and disconnect sockets
     */
    componentWillUnmount() {
        socket_site.off(this.state.socket_listener_uri);
        this._isMounted = false;
    }

    /**
     * Updates state data with fetched data
     * @param {*} data data to insert
     */
    updateStateData(data) {
        sessionStorage.setItem('SiteUpdateStateData', parseInt(sessionStorage.getItem('SiteUpdateStateData'), 10) + 1);
        const formatted_data_objects = formatFeatureSiteObjects(data, this.state.api_endpoint);
        this.setState({
            formatted_data_objects,
        });
        this.props.setLoading(false)
    }

    // Not ideal, data should instead be received from here and passed to single asset
    // Takes the updated asset from single asset and inserts it into its position in asset_objects
    // TODO: hybridOS 2.0
    /**
     * Updates state with data from children component sockets
     * @param {object} object object to insert/update
     */
    updateFromSingleSite(object) {
        // Map for a deep clone
        const updated_formatted_data_objects = this.state.formatted_data_objects.map(
            (formatted_data_object) => (
                formatted_data_object.id === object.id ? object : formatted_data_object
            ),
        );
        this.setState({ formatted_data_objects: updated_formatted_data_objects });
    }

    /**
     * Get data from uri/socket
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
            .catch((error) => this.setState({ error }));
    }

    render() {
        sessionStorage.setItem('SiteRender', parseInt(sessionStorage.getItem('SiteRender'), 10) + 1);
        // eslint-disable-next-line no-shadow
        const { formatted_data_objects, isLoading } = this.state;
        // eslint-disable-next-line react/prop-types
        const { classes } = this.props;
        return (
            <Fragment>
                { formatted_data_objects.map((asset, index) => (
                    <Alert
                        key={`${asset.name}_${index}`}
                        asset={asset}
                        id={asset.id}
                        classes={classes}
                    />
                ))}
                {formatted_data_objects.length > 0
                    ? <Grid container>
                        {formatted_data_objects.map((data_object) => {
                            if (data_object.status.length !== 0
                                && data_object.id !== 'summary') {
                                return (
                                    <SingleSite
                                        key={data_object.id}
                                        data={data_object}
                                        classes={classes}
                                        updateSitePage={this.updateFromSingleSite}
                                    />
                                );
                            }
                            return null;
                        })}
                    </Grid>
                    : null}
            </Fragment>
        );
    }
}
Site.propTypes = {
    classes: PropTypes.object,
};
export default withStyles(LoadingHOC(Site), STYLES_FEATURES);
