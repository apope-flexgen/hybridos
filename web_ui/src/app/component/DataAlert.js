import React from 'react';

import {
    Snackbar,
    Typography,
} from '@mui/material';

import AlertTitle from '@mui/material/AlertTitle';
import MuiAlert from '@mui/material/Alert';

import SocketWrapper from './SocketWrapper';

const DEFAULT_UPDATE_TIME = 1000;

function Alert(props) {
    return <MuiAlert elevation={6} variant="filled" {...props} />;
}

class AlarmAlert extends React.Component {
    render() {
        const { open, onClose, level, alerts, } = this.props;
        let color;
        let bColor;
        let severity;
        switch (level) {
            case "alarms":
                bColor = 'rgba(255, 225, 0, 1.0)';
                color = 'black';
                severity = "warning"
                break;
            case "faults":
                bColor = 'rgba(220, 3, 3, 1.0)';
                color = 'white';
                severity = "error"
                break;
        }

        return (
            <Snackbar
                anchorOrigin={{
                    vertical: 'bottom',
                    horizontal: 'center'
                }}                open={open}
                onClose={onClose}
                TransitionComponent='none'
            >
                <Alert onClose={onClose} severity={severity} style={{ backgroundColor: bColor, color }}>
                    <AlertTitle style={{ fontWeight: 'bold' }}>{level.toUpperCase()}</AlertTitle>
                    {alerts && alerts.map((alarm) => (
                        <Typography>{`${alarm.name}\n`}</Typography>
                    ))}
                    {/* <AlertTitle style={{ fontWeight: 'bold', marginTop: '10px', marginBottom: '0px' }}>{`BMS 1`}</AlertTitle>
                    <Typography>{`Temperature - Low temperature detected\n`}</Typography>
                    <Typography>{`Error - IDK man something is not happy\n`}</Typography>
                    <AlertTitle style={{ fontWeight: 'bold', marginTop: '10px', marginBottom: '0px'  }}>{`BMS 4`}</AlertTitle>
                    <Typography>{`Temperature - Low temperature detected\n`}</Typography>
                    <Typography>{`You should - probably deal with this at some point\n`}</Typography> */}
                </Alert>
            </Snackbar>
        );
    }
}

export default AlarmAlert;