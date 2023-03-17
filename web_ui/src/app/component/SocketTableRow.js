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

class SocketTableRow extends React.Component {
    constructor(props) {
        super(props);

        this.state = {
            value: null,
            updating: true,
            field: props.status.uri.substring(1),
        };
    }

    componentDidUpdate(prevProps) {
        if (prevProps.baseURI !== this.props.baseURI) {
            this.setState({
                value: null,
                updating: true,
                field: this.props.status.uri.substring(1),
            });
        }
    }

    update = (data) => {
        if (this.state.updating) {
            const { field } = this.state;
            const { scalar } = this.props.status;
            data = JSON.parse(data);
            // handling naked vs clothed
            let value = typeof data[field] === 'object' ? data[field].value : data[field];
            if (typeof value === 'number') {
                if (this.props.status.scalar) {
                    value = value / parseFloat(this.props.status.scalar);
                }
                value = Number.parseFloat(value).toFixed(2);
            }
            if (typeof value === 'boolean') {
                value = value ? 'True' : 'False';
            }
            this.setState({
                value: value ? value : this.state.value,
                updating: false,
            }, () => setTimeout(() => this.setState({ updating: true }), DEFAULT_UPDATE_TIME));
        }
    }

    render() {
        const { status, classes, baseURI } = this.props;

        return (
            <SocketWrapper
                baseURI={baseURI}
                sourceURI={status.sourceURI}
                update={this.update}
            >
                <TableRow className={classes.root}>
                    <TableCell style={{ padding: '1em', width: '65%' }}>{status.name}</TableCell>
                    <TableCell style={{ textAlign: 'right', paddingRight: 5, paddingTop: '1em', paddingBottom: '1em', width: '15%' }}>{this.state.value}</TableCell>
                    <TableCell style={{ textAlign: 'left', paddingLeft: 5, paddingTop: '1em', paddingBottom: '1em', width: '15%' }}>{status.units}</TableCell>
                </TableRow>
            </SocketWrapper>
        );
    }
}

export default withStyles(SocketTableRow, STYLES_TABLE_ROW);
