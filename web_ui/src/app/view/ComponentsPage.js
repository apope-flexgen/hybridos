/* eslint-disable react/prop-types */
/* eslint-disable camelcase */
import React, { Fragment } from 'react';
import { withStyles } from 'tss-react/mui';
import Box from '@mui/material/Box';
import Grid from '@mui/material/Grid';
import CircularProgress from '@mui/material/CircularProgress';
import SingleComponent from '../component/SingleComponent';
import LoadingHOC from '../component/LoadingHOC';
import { STYLES_FEATURES } from '../styles';
import {
    isLoading,
    alertComponentsEventEmitter,
} from '../../AppConfig';
import {
    siteConfiguration,
    socket_components,
} from '../../AppAuth';
import classNames from 'classnames';

/**
 * Component for displaying components (not react)
 */
class ComponentsPage extends React.PureComponent {
    /**
     * Iniitalizing state
     * @param {*} props 
     */
    constructor(props) {
        super(props);
        this.state = {
            componentIDs: [],
            socket: socket_components,
            lastFimsMessage: {},
            inspectorComponentsName: '',
        };
        this.props.setLoading(isLoading || true)
    }

    // we are using React 16.4.1 - before the deprecation referenced below
    // eslint-disable-next-line react/no-deprecated
    /**
     * Setup components or control_cabinet metadata and connect sockets
     */
    UNSAFE_componentWillMount() {
        const inspectorComponentsName = siteConfiguration.inspectorComponentsName ? siteConfiguration.inspectorComponentsName : 'hybridos';
        let componentsMetadata;
        if (this.props.type === 'components') {
            componentsMetadata = siteConfiguration.metadata && siteConfiguration.metadata.components
                ? siteConfiguration.metadata.components : [];
        } else if (this.props.type === 'control_cabinet') {
            componentsMetadata = siteConfiguration.metadata
                && siteConfiguration.metadata.control_cabinet
                ? siteConfiguration.metadata.control_cabinet : [];
        }
        const componentIDsTemp = [];
        const componentIDs = [];
        const statusIDs = [];
        const controlIDs = [];
        if (componentsMetadata) {
            componentsMetadata.forEach((component) => {
                componentIDsTemp.push(component.id);
                this.setState({ [component.id]: component });
                socket_components.on(`/components/${component.id}`, (data) => this.updateStateData(JSON.parse(data), component.id, component.name));
                componentIDs.push(component.id);
                component.statuses.forEach((status) => {
                    statusIDs.push(`${component.id}/${status.id}`);
                });
                component.controls.forEach((control) => {
                    controlIDs.push(`${component.id}/${control.id}`);
                });
            });
        }
        this.setState({ // keeps a list of components and a
            // list of statusIDs we'll accept from fims
            componentIDs: componentIDsTemp,
            statusIDs,
            controlIDs,
            inspectorComponentsName,
        });
    }

    /**
     * Turn off isLoading if in development
     */
    componentDidMount() {
        // if we don't get a fimsMessage updating state data then we may be in
        // development and/or there may be no data coming in for the components
        // we are looking at. In that case, we'll turn off isLoading
        setTimeout(() => {
            this.props.setLoading(false)
        }, 3000);
    }

    /**
     * Disconnect sockets
     */
    componentWillUnmount() {
        this.state.componentIDs.forEach((component) => {
            this.state.socket.off(`/components/${component}`);
        });
    }

    /**
     * Update with data from sockets
     * @param {object} fimsMessage message from fims
     * @param {*} theComponentID ID of component to update
     */
    updateStateData(fimsMessage, theComponentID) {
        sessionStorage.setItem('ComponentsPageUpdateStateData', parseInt(sessionStorage.getItem('ComponentsPageUpdateStateData'), 10) + 1);
        if (fimsMessage !== undefined) {
            if (JSON.stringify(fimsMessage) !== JSON.stringify(this.state.lastFimsMessage)) {
                this.setState({
                    lastFimsMessage: fimsMessage,
                });
                const { statusIDs } = this.state;
                const { controlIDs } = this.state;
                const theComponentToUpdate = this.state[theComponentID];
                Object.keys(fimsMessage).map((messagePart) => {
                    if (messagePart === 'faults' || messagePart === 'alarms') {
                        if (this.state[theComponentID].faults !== fimsMessage.faults
                            && this.state[theComponentID].alarms !== fimsMessage.alarms) {
                            alertComponentsEventEmitter.emit('removeAlertData', fimsMessage);
                        }
                        if ((fimsMessage.faults !== undefined && fimsMessage.faults.length > 0)
                            || (fimsMessage.alarms !== undefined
                                && fimsMessage.alarms.length > 0)) {
                            alertComponentsEventEmitter.emit('addAlertData', fimsMessage);
                        }
                    } else {
                        if (statusIDs.includes(`${theComponentID}/${messagePart}`)) {
                            theComponentToUpdate.statuses.map((status, i) => {
                                if (status.id === messagePart) {
                                    // eslint-disable-next-line max-len
                                    theComponentToUpdate.statuses[i].value = fimsMessage[messagePart];
                                }
                                return null;
                            });
                        }
                        if (controlIDs.includes(`${theComponentID}/${messagePart}`)) {
                            theComponentToUpdate.controls.map((control, i) => {
                                if (control.id === messagePart) {
                                    // eslint-disable-next-line max-len
                                    theComponentToUpdate.controls[i].value = fimsMessage[messagePart];
                                }
                                return null;
                            });
                        }
                    }
                    this.setState({
                        [theComponentID]: theComponentToUpdate,
                    });
                    this.props.setLoading(false)
                    return null;
                });
            }
        }
    }

    render() {
        const { classes } = this.props;
        sessionStorage.setItem('ComponentsPageRender', parseInt(sessionStorage.getItem('ComponentsPageRender'), 10) + 1);
        if (this.props.type !== 'control_cabinet' && this.props.asset_objects[0]) { // then we're displaying something in an asset like solar
            if ((this.props.asset_objects[0].uri).includes('/solar/') && (this.state.componentIDs.length > 0)) { // as of 9/2019, we only display components with Solar assets, and the only components we display are Met Station and Tracker
                return (<>
                    {!this.state.isLoading && this.state.componentIDs.map((component, i) => {
                        if (component === 'met_station' || component === 'tracker_summary') {
                            const theComponent = this.state[component];
                            return (theComponent !== undefined
                                ? <SingleComponent key={i} component={theComponent} /> : null);
                        }
                        return null;
                    })}
                </>
                );
            }
            return null;
        }
        return (<>
            {!this.state.isLoading &&
                 this.state.componentIDs.map((component) => {
                    const theComponent = this.state[component];
                    if (theComponent) {
                        return (<Fragment key={theComponent.id}>
                            <Grid container>
                                <SingleComponent component={theComponent} />
                            </Grid>
                        </Fragment>);
                    }
                    return (null);
                })
            }
        </>
        );
    }
}
export default withStyles(LoadingHOC(ComponentsPage), STYLES_FEATURES);
