import React from 'react';
import PropTypes from 'prop-types';
import Table from '@mui/material/Table';
import TableBody from '@mui/material/TableBody';
import TableCell from '@mui/material/TableCell';
import TableRow from '@mui/material/TableRow';

/**
 * Handles click events
 * @param {*} event
 */
function handleClick(event) {
    if (document.getElementById('inspector') !== null) {
        console.log(this);
        sessionStorage.setItem('lastReference', this);
    }
}

/**
 * Component for rendering status table
 * @param {*} props
 */
const StatusTable = (props) => {
    // eslint-disable-next-line react/prop-types
    const { status, classes } = props;
    return (
        <>
            <Table className={classes.table}>
                <TableBody>
                    {Object.keys(status).map((item, i) => {
                        if (status[item].displayValue !== undefined) {
                            let theDisplayValue = status[item].displayValue === null ? 'null'
                                : (status[item].displayValue.toString()).charAt(0).toUpperCase()
                                + (status[item].displayValue.toString()).slice(1);
                            // If there's a null value, we want it to display as such in
                            // the UI - that might be an indicator of an error or unexpected
                            // value in whatever repo or process produced the value.
                            if (status[item].unit === 'int') {
                                theDisplayValue = parseInt(theDisplayValue, 10);
                                // parseInt cuts off any decimal. ".toFixed(0)" could be used
                                // to round any decimal
                                status[item].unitPrefix = '';
                                status[item].unit = '';
                            }
                            return (
                                <TableRow data-cy={status[item].asset_id || status[item].api_endpoint} key={i} onClick={handleClick.bind(`/${status[item].base_uri}/${status[item].category}${status[item].asset_id ? `/${status[item].asset_id}` : ''} '{"${status[item].api_endpoint || status[item].id}": ${status[item].value}}'`)}>
                                    <TableCell style={{ padding: '1em', width: '65%' }} >{status[item].name}</TableCell>
                                    <TableCell style={{
                                        textAlign: 'right', paddingRight: 5, paddingTop: '1em', paddingBottom: '1em', width: '15%',
                                    }}>{theDisplayValue}</TableCell>
                                    <TableCell style={{
                                        textAlign: 'left', paddingLeft: 5, paddingTop: '1em', paddingBottom: '1em', width: '15%',
                                    }}>{status[item].unitPrefix}{status[item].unit}</TableCell>
                                </TableRow>
                            );
                        }
                        return null;
                    })}
                </TableBody>
            </Table>
        </>
    );
};
StatusTable.propTypes = {
    classes: PropTypes.object,
    status: PropTypes.array,
};
export default StatusTable;
