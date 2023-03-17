/* eslint-disable camelcase */
import React from 'react';
import PropTypes from 'prop-types';
import { withStyles } from 'tss-react/mui';
import Paper from '@mui/material/Paper';
import Typography from '@mui/material/Typography';
import Warning from '@mui/icons-material/Warning';
import IconButton from '@mui/material/IconButton';
import { Link } from 'react-router-dom';
import Error from '@mui/icons-material/Error';
import Grid from '@mui/material/Grid';
import { EVENTS_PAGE_PATH } from '../../../AppConfig';
import { STYLES_ALERT } from '../../styles';

/**
 * Component for rendering alerts
 */
class Alert extends React.PureComponent {
    // Determines if alert should be displayed at all
    /**
     * Determines if any faults or alarms exist
     */
    shouldDisplayAlert = (asset) => {
        if (asset.id !== 'summary') {
            if (asset.faults && asset.faults.length > 0) {
                for (let i = 0; i < asset.faults.length; i += 1) {
                    if (asset.faults[i].value > 0) return true;
                }
            }
            if (asset.alarms && asset.alarms.length > 0) {
                for (let i = 0; i < asset.alarms.length; i += 1) {
                    if (asset.alarms[i].value > 0) return true;
                }
            }
        }

        return false;
    }

    // Determines if color theme should be faulted(red) or not
    /**
     * Determines if there is an active fault
     * @param {object} faults faults to check
     */
    checkForActiveFaults = (faults) => {
        if (faults && faults.length > 0) {
            for (let i = 0; i < faults.length; i += 1) {
                if (faults[i].value > 0) {
                    return true;
                }
            }
        }

        return false;
    }

    render() {
        const { asset, classes } = this.props;
        const hasActiveFaults = this.checkForActiveFaults(asset.faults);
        // Sets color scheme for faulted(red) or not
        const backgroundColor = hasActiveFaults ? '#D43F3A' : '#FCD116';
        const color = hasActiveFaults ? 'white' : 'black';

        if (this.shouldDisplayAlert(asset)) {
            return (
                <Paper style={{ backgroundColor }} className={[classes.root].join(' ')} elevation={5}>
                    <Typography style={{ color }} variant='h5' component="p">{asset.name}</Typography>
                    <Grid container wrap='nowrap'>
                        <Grid item md={6} lg={6}>
                            <Typography style={{ color }} component="p">
                                Faults:
                            </Typography>
                            {asset.faults && asset.faults.map((fault, index) => {
                                if (fault.value > 0) {
                                    return (
                                        <React.Fragment key={`${fault.name}_${index}`}>
                                            <Typography style={{ color }} component="p">
                                                <IconButton component={Link} to={EVENTS_PAGE_PATH} size="large">
                                                    <Error style={{ color }} />
                                                </IconButton>
                                                {fault.name}
                                            </Typography>
                                            {fault.options && fault.options.map((option) => (
                                                <Typography key={`${fault.name}_${option.name}`} style={{ color }} component="p" className={classes.leftText}>
                                                    • {option.name}
                                                </Typography>
                                            ))}
                                        </React.Fragment>
                                    );
                                }
                                return null;
                            })}
                        </Grid>
                        <Grid item md={6} lg={6}>
                            <Typography style={{ color }} component="p">
                                Alarms:
                            </Typography>
                            {asset.alarms && asset.alarms.map((alarm, index) => {
                                if (alarm.value > 0) {
                                    return (
                                        <React.Fragment key={`${alarm.name}_${index}`}>
                                            <Typography style={{ color }} component="p">
                                                <IconButton component={Link} to={EVENTS_PAGE_PATH} size="large">
                                                    <Warning style={{ color }} />
                                                </IconButton>
                                                {alarm.name}
                                            </Typography>
                                            {alarm.options && alarm.options.map((option) => (
                                                <Typography key={`${alarm.name}_${option.name}`} style={{ color }} component="p" className={classes.leftText}>
                                                    • {option.name}
                                                </Typography>
                                            ))}
                                        </React.Fragment>
                                    );
                                }
                                return null;
                            })}
                        </Grid>
                    </Grid>
                </Paper>
            );
        }

        return null;
    }
}

Alert.propTypes = {
    classes: PropTypes.object,
    asset: PropTypes.object,
};

export default withStyles(Alert, STYLES_ALERT);
