/* eslint-disable react/prop-types */
import React from 'react';
import Typography from '@mui/material/Typography';
import Divider from '@mui/material/Divider';
import Grid from '@mui/material/Grid';
import List from '@mui/material/List';

import TabAlarms from './TabAlarms';
import Control from './Control';

/**
 * Status line display component
 * @param {object} status status to display
 * @param {number} index key
 * @param {number} length used for divider logic
 */
function renderStatusLine(status, index, length) {
    return (
        <React.Fragment key={status.id}>
            <Grid container spacing={3}>
                <Grid item xs={status.id === 'status' ? 2 : 8}>
                    <Typography style={{ fontWeight: 'bold', letterSpacing: 1 }}>{status.name}</Typography>
                </Grid>
                <Grid item xs={status.id === 'status' ? 8 : 2}>
                    <Typography align='right'>
                        {typeof status.value === 'number'
                            ? Math.round(status.value * 100) / 100
                            : status.value
                        }
                    </Typography>
                </Grid>
                <Grid item xs={2}>
                    <Typography style={{ fontWeight: 'bold' }}>{status.unit}</Typography>
                </Grid>
            </Grid>
            {index !== length - 1
                && <Divider style={{ width: '100%', marginBottom: '4px', marginTop: '2px' }} />
            }
        </React.Fragment>
    );
}

/**
 * Base tab component, controlled by tabs
 */
class GridTab extends React.PureComponent {
    render() {
        const { data, tab } = this.props;

        return (
            <React.Fragment>
                {data
                    ? <React.Fragment>
                        <Typography variant='h5' style={{ fontWeight: 'bold' }}>{this.props.component} {tab}</Typography>
                        <Divider style={{ width: '60%', marginBottom: '2em' }} />
                        {tab === 'status'
                            && data.status.map(
                                (status, index) => renderStatusLine(status, index,
                                    data.status.length),
                            )
                        }
                        {tab === 'alarms'
                            && <List style={{ width: '100%', paddingTop: '0px' }}>
                                {/* {mockAlarms.map((alarm, index) => ( */}
                                {data.alarms.map((alarm, index) => (
                                    alarm.value > 0 && <TabAlarms alarm={alarm} key={index} />
                                ))}
                            </List>
                        }
                        {tab === 'controls'
                            && data.controls.map(
                                (control, index) => <Control control={control} key={index} />,
                            )
                        }
                    </React.Fragment>
                    : null
                }
            </React.Fragment>
        );
    }
}

export default GridTab;
