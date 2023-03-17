import React from 'react';
import SocketWrapper from './SocketWrapper';
import { withStyles } from 'tss-react/mui';

import { Button } from '@mui/material';

import { STYLES_SOCKET_GRID_BUTTON } from '../styles';

const DEFAULT_UPDATE_TIME = 100;

/*
    Props
        selectedItem: number
        selectItem: function(item) {
            selects item in parent
        }
        item: number
        px: string
        baseURI: string
        sourceURI: string
*/
class SocketGridButton extends React.Component {
    constructor(props) {
        super(props);

        this.state = {
            alarms: [],
            faults: [],
            isMaint: false,
            alarmFields: props.alarmFields,
            faultFields: props.faultFields,
            updating: true
        }
    }

    componentDidUpdate(prevProps) {
        if (prevProps.baseURI !== this.props.baseURI) {
            this.setState({
                alarmFields: this.props.alarmFields,
                faultFields: this.props.faultFields,
                updating: true
            })
        }
    }

    update = (data) => {
        if (this.state.updating) {
            data = JSON.parse(data);

            if(data.maint_mode) {
                if(data.maint_mode.value) {
                    this.setState({isMaint: true})
                } else if (!data.maint_mode.value || data.maint_mode.value === null) {
                    this.setState({ isMaint: false})
                }
            }

            const { alarmFields, faultFields } = this.state;
            let alarms = [];
            if (alarmFields && data) {
                for (const field of alarmFields) {
                    alarms = (data[field] && data[field].options) ? alarms.concat(data[field].options) : alarms;
                }
            }
            let faults = [];
            if (faultFields && data) {
                for (const field of faultFields) {
                    faults = (data[field] && data[field].options) ? faults.concat(data[field].options) : faults;
                }
            }

            /*
                TODO: Consider using flatmap
                const alarms = alarmFields.flatMap( (field) => {
                    if ( data && data[field] && data[field].options ) {
                        return data[field];
                    } else {
                        return [];
                    }
                );
            */

            (this.props.item === this.props.selectedItem) && this.props.updateFromButton(data, alarms, faults);
    
            this.setState({
                alarms,
                faults,
                updating: false,
            }, () => setTimeout(() => this.setState({ updating: true }), DEFAULT_UPDATE_TIME));
        }
    }

    render() {
        const { baseURI, sourceURI, selectItem, selectedItem, item, px, classes } = this.props;
        const { alarms, faults, isMaint} = this.state;
        let secondaryColor, color = null;

        if(isMaint) {
            if(selectedItem === item) {
                color = 'white'
                secondaryColor = 'orange';
            } else {
                color = 'orange';
                secondaryColor = null;
            }
        } else {
            secondaryColor = null;
            color = null;
        }

        return (
            <SocketWrapper
                baseURI={baseURI}
                sourceURI={sourceURI}
                update={this.update}
            >
                <Button
                    className={`${item !== 'summary' && item !== 'allcontrols' && classes.root} ${alarms.length !== 0 && classes.alarm} ${faults.length !== 0 && classes.fault}`}
                    variant={selectedItem === item ? 'outlined' : 'contained'}
                    style={px ? { maxWidth: px, maxHeight: px, minWidth: px, minHeight: px, backgroundColor: color, color: secondaryColor, borderColor: secondaryColor } : {}}
                    onClick={() => selectItem(item)}
                >
                    {item}
                </Button>
            </SocketWrapper>
        );
    }
}

export default withStyles(SocketGridButton, STYLES_SOCKET_GRID_BUTTON);