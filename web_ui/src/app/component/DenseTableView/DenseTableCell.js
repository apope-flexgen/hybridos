/* eslint-disable no-unused-expressions */
/* eslint-disable no-underscore-dangle */
/* eslint-disable camelcase */
/* eslint-disable react/prop-types */
import React from 'react';
import { withStyles } from 'tss-react/mui';
import { DENSE_TABLE_CELL } from '../../styles';

import {
    isLoading,
    getDataForURI,
    formatAssetObjects,
} from '../../../AppConfig';
import {
    socket_features,
    socket_assets,
    socket_site,
    socket_sites,
    socket_components,
} from '../../../AppAuth';

import TableCell from '@mui/material/TableCell';

// TODO: Refactor out socket logic, use socketWrapper
class DenseTableCell extends React.Component {
    /**
     * Given a template can have multiple source URIs, we must have a socket open for each item-value combination
     */
    constructor(props) {
        super(props);
        let setupProps;
        let socketToBeUsed;
        switch (props.sourceURI) {
            case '/site':
                socketToBeUsed = socket_site;
                break;
            case '/sites':
                socketToBeUsed = socket_sites;
                break;
            case '/assets':
                socketToBeUsed = socket_assets;
                break;
            case '/components':
                socketToBeUsed = socket_components;
                break;
            case '/features':
                socketToBeUsed = socket_features;
                break;
            default:
                console.log('invalid source URI');
                break;
        }
        setupProps = {
            socket_listener_uri: props.sourceURI + props.baseURI,
            api_endpoint: props.sourceURI + props.baseURI,
            socket: socketToBeUsed,
        };
            
           
        this.state = {
            isLoading: isLoading || true,
            data: [],
        };
        this._isMounted = false;
        this.state = {
            selected: null,
            value: '---',
            ...this.state,
            ...setupProps,
        };

    }

    /**
     * Connect socket based on setup props
     */
     componentDidMount() {
        this._isMounted = true;
        this.manageSocket('connect');
    }

    /**
     * Disconnect socket
     */
    componentWillUnmount() {
        this._isMounted = false;
        this.manageSocket('disconnect');
    }

    /**
     * Turns on and off socket
     * @param {string} - connect or disconnect
     */
    manageSocket(type) {
        if (this.state.socket) {
            let uri = `${this.props.sourceURI}${this.props.baseURI}`;
            if (type === 'connect') this.state.socket.on(uri, (data) => this.updateStateData(data));
            if (type === 'disconnect') this.state.socket.off(uri);
        }
    }

    /**
     * Finds the specified field for the cell we want and scales it, formats it appropriately
     * Ex. /ess/ess_1/soc (soc is the field)
     */
    updateStateData(data) {
        data = JSON.parse(data)
        let field = this.props.uri.substring(1)
        let value = typeof data[field] === 'object' ? data[field].value : data[field];
        if (typeof value === 'number') {
            if (this.props.scalar) {
                value = value / parseFloat(this.props.scalar);
            }
            value = Number.parseFloat(value).toFixed(2);
        }
        if (typeof value === 'boolean') {
            value = value ? 'True' : 'False';
        }
        if (value == null) {
            value = '---';
        }
        this.setState({
            isLoading: false,
            value: value
        });
    }

    render() {
        return(
            <TableCell>
                {this.state.value}
            </TableCell>
        )
    }
}

export default withStyles(DenseTableCell, DENSE_TABLE_CELL);
