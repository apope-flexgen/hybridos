import React from 'react';

import SocketWrapper from './SocketWrapper';

import {
    TableRow,
    TableCell,
} from '@mui/material';

import { withStyles } from 'tss-react/mui';
import { STYLES_TABLE_ROW } from '../styles';

const DEFAULT_UPDATE_TIME = 1000;

/*
    Props
        status: object {
            uri: string
            scalar: number
            sourceURI: string
            name: string
            units: string
        }
        baseURI: string
*/

class DataTableRow extends React.Component {
    formatData = (data) => {
        if (data !== null) {
            let value = typeof data === 'object' ? data.value : data;
            if (typeof value === 'number') {
                if (this.props.status.scalar) {
                    value = value / parseFloat(this.props.status.scalar);
                }
                value = Number.parseFloat(value).toFixed(2);
            }
            if (typeof value === 'boolean') {
                value = value ? 'True' : 'False';
            }
            return value;
        }
        
        return '';
    }

    render() {
        const { status, classes } = this.props;

        return (
            <TableRow className={classes.root}>
                <TableCell style={{ padding: '1em', width: '65%' }}>{status.name}</TableCell>
                <TableCell style={{ textAlign: 'right', paddingRight: 5, paddingTop: '1em', paddingBottom: '1em', width: '15%' }}>
                    {this.formatData(this.props.data)}
                </TableCell>
                <TableCell style={{ textAlign: 'left', paddingLeft: 5, paddingTop: '1em', paddingBottom: '1em', width: '15%' }}>{status.units}</TableCell>
            </TableRow>
        );
    }
}

export default withStyles(DataTableRow, STYLES_TABLE_ROW);
