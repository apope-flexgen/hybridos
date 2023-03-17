/* eslint-disable no-unused-expressions */
/* eslint-disable no-underscore-dangle */
/* eslint-disable camelcase */
import React, { Fragment } from 'react';
import PropTypes from 'prop-types';
import { withStyles } from 'tss-react/mui';
import Grid from '@mui/material/Grid';
import SingleFeature from '../component/SingleFeature';
import { STYLES_FEATURES } from '../styles';
import LoadingHOC from '../component/LoadingHOC';
import {
    isLoading,
    getDataForURI,
    formatFeatureSiteObjects,
} from '../../AppConfig';
import {
    socket_features,
} from '../../AppAuth';

/**
 * Component for rendering features
 */
class Features extends React.PureComponent {
    constructor(props) {
        super(props);
        this.state = {
            formatted_data_objects: [],
            api_endpoint: 'features',
            socket_listener_uri: '/features',
        };
        this._isMounted = false;
        this.props.setLoading(isLoading || true)
    }

    // we are using React 16.4.1 - before the deprecation referenced below
    // eslint-disable-next-line react/no-deprecated
    /**
     * Connect sockets
     */
    UNSAFE_componentWillMount() {
        socket_features.on(this.state.socket_listener_uri, (data) => {
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
        socket_features.off(this.state.socket_listener_uri);
        this._isMounted = false;
    }

    /**
     * Updates state data with fetched data
     * @param {*} data data to insert
     */
    updateStateData(data) {
        sessionStorage.setItem('FeaturesUpdateStateData', parseInt(sessionStorage.getItem('FeaturesUpdateStateData'), 10) + 1);
        const formatted_data_objects = formatFeatureSiteObjects(data, this.state.api_endpoint);
        this.setState({
            formatted_data_objects,
        });
        this.props.setLoading(false)
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
            .catch((error) => this.setState({ error }));
    }

    render() {
        sessionStorage.setItem('FeaturesRender', parseInt(sessionStorage.getItem('FeaturesRender'), 10) + 1);
        // eslint-disable-next-line no-shadow
        const { formatted_data_objects } = this.state;
        // eslint-disable-next-line react/prop-types
        return (
            <Fragment>
                {  formatted_data_objects.length > 0
                    ? <Fragment>
                        <Grid container>
                            {formatted_data_objects.map((data_object) => {
                                if (data_object.status.length !== 0
                                    && data_object.id !== 'summary') {
                                    return (<SingleFeature key={data_object.id}
                                        data={data_object} />);
                                }
                                return null;
                            })}
                        </Grid>
                    </Fragment>
                    : null}
            </Fragment>
        );
    }
}
Features.propTypes = {
    classes: PropTypes.object,
};
export default withStyles(LoadingHOC(Features), STYLES_FEATURES);
