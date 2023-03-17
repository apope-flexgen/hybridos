import React from 'react';

import {
    socket_features,
    socket_assets,
    socket_site,
    socket_sites,
    socket_components,
} from '../../AppAuth';

/*
    Props
        sourceURI: string
        baseURI: string
        update: function()
            - Parent update function
*/

class SocketWrapper extends React.PureComponent {
    constructor(props) {
        super(props);

        let socket;
        switch (props.sourceURI) {
            case '/site':
                socket = socket_site;
                break;
            case '/sites':
                socket = socket_sites;
                break;
            case '/assets':
                socket = socket_assets;
                break;
            case '/components':
                socket = socket_components;
                break;
            case '/features':
                socket = socket_features;
                break;
            default:
                console.log('invalid sourceURI');
                break;
        }

        this.state = { socket };
    }

    componentDidMount() {
        this.manageSocket('connect');
    }

    componentDidUpdate(prevProps) {
        if (prevProps.baseURI !== this.props.baseURI) {
            let oldURI = `${prevProps.sourceURI}${prevProps.baseURI}`;
            this.manageSocket('disconnect', oldURI);
            this.manageSocket('connect');
        }
    }

    componentWillUnmount() {
        this.manageSocket('disconnect');
    }

    manageSocket = (type, oldURI) => {
        if (this.state.socket) {
            const { sourceURI, baseURI } = this.props;
            let uri = `${sourceURI}${baseURI}`;
            if (type === 'connect') this.state.socket.on(uri, (data) => this.props.update(data));
            if (type === 'disconnect') {
                this.state.socket.off(oldURI ? oldURI : uri);
            }
        }
    }

    render() {
        return (
            <React.Fragment>
                {this.props.children}
            </React.Fragment>
        );
    }
}

export default SocketWrapper;