/* eslint-disable no-unused-expressions */
/* eslint-disable no-underscore-dangle */
/* eslint-disable camelcase */
/* eslint-disable react/prop-types */
import React from 'react';
import { withStyles } from 'tss-react/mui';
import { DENSE_TABLE_ROW } from '../../styles';

import TableCell from '@mui/material/TableCell';
import TableRow from '@mui/material/TableRow';
import DenseTableCell from './DenseTableCell';

class DenseTableViewRow extends React.Component {
    /**
     * This class creates the row for one item (/ess_1 for example) and asks DenseTableCell to fetch the data for each status
     */
    constructor(props) {
        super(props);
    }

    // TODO: Refactor, _ismounted is probably not necessary
     componentDidMount() {
        this._isMounted = true;
    }

    componentWillUnmount() {
        this._isMounted = false;
    }


    render() {
        return(
            <TableRow style ={ this.props.index%2? { backgroundColor : "white" }:{ backgroundColor : "#e3e7fa" }}>
                <TableCell>
                    {this.props.rowName}
                </TableCell>
                {this.props.desiredFields.map((value) => (
                    <DenseTableCell
                        baseURI={this.props.baseURI}
                        scalar = {value.scalar}
                        uri = {value.uri}
                        sourceURI={value.sourceURI}/>))}
                    
                
            </TableRow>
        )
    }
}

export default withStyles(DenseTableViewRow, DENSE_TABLE_ROW);
