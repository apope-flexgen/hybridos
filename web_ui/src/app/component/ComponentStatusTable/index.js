import React from 'react';
import PropTypes from 'prop-types';
import Table from '@mui/material/Table';
import TableBody from '@mui/material/TableBody';
import TableCell from '@mui/material/TableCell';
import TableRow from '@mui/material/TableRow';

/**
 * Capitalizes string
 * @param {string} str1 
 */
function capitalize(str1) {
    return str1.charAt(0).toUpperCase() + str1.slice(1);
}

/**
 * Formats value based on type
 * @param {*} value value to format
 */
function formatValue(value) {
    if (typeof value === 'number') return value.toFixed(2);
    if (typeof value === 'object') { // looks like an array in FIMS
        let theLines = '';
        value.forEach((line) => {
            theLines += `${capitalize(line.string.toString())} `;
            // the extra space at the end of the string above allows
            // additional lines to break to the next display line
            // (assuming it's a long enough word line "Running"). The
            // only other way to let this happen under material.ui's
            // rules would be to use their "dangerouslySetInnerHTML"
        });
        return theLines;
    }
    return capitalize(value.toString());
}

/**
 * Handles selection
 * @param {*} event 
 */
function handleClick(event) {
    if (document.getElementById('inspector') !== null) {
        console.log(this);
        sessionStorage.setItem('lastReference', this);
    }
}

const ComponentStatusTable = (props) => {
    // eslint-disable-next-line react/prop-types
    const { status, componentID } = props;
    if (status !== undefined) {
        let theDisplayValue = status.value === null ? 'null' : status.value;
        // If there's a null value, we want it to display as such in
        // the UI - that might be an indicator of an error or unexpected
        // value in whatever repo or process produced the value. Note
        // that the typeof of null is "object" whereas 'null' is a "string"
        theDisplayValue = formatValue(theDisplayValue);
        if (status.unit === 'int') {
            theDisplayValue = parseInt(theDisplayValue, 10);
            // parseInt cuts off any decimal. ".toFixed(0)" could be used
            // to round any decimal
            status.unitPrefix = '';
            status.unit = '';
        }
        return (
            <>
                <Table>
                    <TableBody>
                        < TableRow onClick={handleClick.bind(`${componentID} '{"${status.id}": ${status.value}}'`)}>
                            <TableCell style={{ width: '65%' }}>{status.name}</TableCell>
                            <TableCell style={{ textAlign: 'right', paddingRight: 5, width: '15%' }}>{theDisplayValue}</TableCell>
                            <TableCell style={{ textAlign: 'left', paddingLeft: 5, width: '15%' }}>{status.unitPrefix}{status.unit}</TableCell>
                        </TableRow>
                    </TableBody>
                </Table>
            </>
        );
    }
    return null;
};
ComponentStatusTable.propTypes = {
    classes: PropTypes.object,
    status: PropTypes.object,
};
export default ComponentStatusTable;
