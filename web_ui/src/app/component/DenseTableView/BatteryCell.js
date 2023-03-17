/* eslint-disable no-unused-expressions */
/* eslint-disable no-underscore-dangle */
/* eslint-disable camelcase */
/* eslint-disable react/prop-types */
import React from 'react';
import Paper from '@mui/material/Paper';

import { withStyles } from 'tss-react/mui';
import { BATTERY_CELL } from '../../styles';

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

// const BATTERY_RED = '#FF4949';
// const BATTERY_LIGHT_RED = '#ffc9c9';
// const BATTERY_YELLOW ='#FFF503';
// const BATTERY_LIGHT_YELLOW = '#fffdcc';
const BATTERY_GREEN ='#84F314';
const BATTERY_LIGHT_GREEN ='#c7ff8f';

// TODO: Refactor out socket logic, use socketWrapper
class BatteryCell extends React.Component {
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
            percent: 0
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
        let field = this.props.desiredField.substring(1)
        let value = data[field].value
        value = Number.parseFloat(value).toFixed(2);
        
        this.setState({
            isLoading: false,
            percent: value
        });
    }

    /**
     * Returns a battery cell with the appropriate percentage for the battery soc
     */
    render() {
        let percent = this.state.percent
        let color = BATTERY_GREEN;
        let lightColor = BATTERY_LIGHT_GREEN;
       
        let height = (percent/100) * 150
        return(
            <div style = {{width: '60px', textAlign: 'center', alignSelf: 'flex-end',}}>
            <Paper style={{marginBottom: `${percent < 5 ? '15px' : '0px'}`, display: 'flex', flexWrap: 'wrap', backgroundColor: lightColor, width:'60px', height:'150px', borderRadius: '0px'}}>
                <Paper style={{backgroundColor: color, width: '100%', height: height, boxShadow: 'none', textAlign: 'center', fontSize: '16px', alignSelf: 'flex-end', borderRadius: '0px'}}>{percent}%</Paper>
            </Paper>
            {this.props.rowName}
            </div>
        )
    }
}

export default withStyles(BatteryCell, BATTERY_CELL);
