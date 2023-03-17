import React from 'react';
import { withStyles } from 'tss-react/mui';
import { STYLES_TABLE_VIEW } from '../../styles';
import Paper from '@mui/material/Paper';
import Table from '@mui/material/Table';
import TableBody from '@mui/material/TableBody';
import TableCell from '@mui/material/TableCell';
import TableContainer from '@mui/material/TableContainer';
import TableHead from '@mui/material/TableHead';
import TableRow from '@mui/material/TableRow';
import Typography from '@mui/material/Typography'
import DenseTableViewRow from './DenseTableViewRow';
import BatteryCell from './BatteryCell';

class TableView extends React.PureComponent {
    constructor(props) {
        super(props);
        if (!props.info.info.isTemplate) {
            let newObject = {};
            newObject["name"] = props.info.info.name;
            newObject['uri'] = props.info.info.baseURI;
            props.info.info.items = [newObject];
        }
        

        // TODO: Unnecessary props into state if not changed
        this.state = {
            items: props.info.info.items,
            baseURI: props.info.info.baseURI, //assets
            batteryViewURI: props.info.info.batteryViewURI,
            batteryViewSourceURI: props.info.info.batteryViewSourceURI,
            rows: props.info.status, //ess/ess_1, ess/ess_2
            fullCard: props.info,
            showBatteryView: props.info.info.batteryView
        };
    }

    generateTable() {
        let desiredFields = [];
        let desiredURIs = [];

        // TODO: Refactor to arrays.map or potentially arrays.flatmap()
        for (let i = 0; i < this.state.rows.length; i++) {
            if (this.state.rows[i].units != null && this.state.rows[i].units != '') {
                desiredFields.push(this.state.rows[i].name + " (" + this.state.rows[i].units + ")");
            } else {
                desiredFields.push(this.state.rows[i].name);
            }
            let newObject = {};
            newObject['uri'] = this.state.rows[i].uri;
            newObject['sourceURI'] = this.state.rows[i].sourceURI;
            newObject['scalar'] = this.state.rows[i].scalar;
            desiredURIs.push(newObject);

        }
        // TODO: Refactor to non-magic numbers, hopefully with withstyles
        let height = 54 * this.state.fullCard.info.items.length + 54 // 54
        height = Math.min(550, height)
        let width = this.state.fullCard.info.items.length * 75 // 75
        return(<>
        <Typography
            variant="h6">
            {this.state.fullCard.info.name}
        </Typography>
        {this.state.showBatteryView &&
            
// visible
            <TableContainer component={Paper} style={{overflowX: 'auto', boxShadow: 'none', textAlign: 'center', alignItems: 'center'}}>
                <Table  style={{ width: `${this.state.items.length * 90}px`, tableLayout: "fixed", marginLeft: 'auto', marginRight: 'auto'}}>
                    <TableHead>
                        <TableRow style={{backgroundColor: '#f9f9f9', }}>
                             {this.state.items.map((row, index) => {
                                return(
                                    <TableCell>
                                    <BatteryCell
                                        rowName = {row.name}
                                        sourceURI={this.props.info.info.batteryViewSourceURI}
                                        baseURI = {row.uri}
                                        index={index}
                                        desiredField = {this.state.batteryViewURI}
                                    /></TableCell>)})}
                        </TableRow>
                    </TableHead>
                </Table>
            </TableContainer>
        }
        {desiredURIs.length > 0 &&
        <div style = {{ marginBottom: '5%', overflowY: `${height == 550 ? 'scroll' : 'visible'}`}}>
        
        <TableContainer component={Paper} style={{overflowX: 'auto', boxShadow: 'none' }}>
            <Table stickyHeader  style={{  tableLayout: "fixed" }}>
                <TableHead >
                    <TableRow >
                        <TableCell style={{backgroundColor: 'white', fontWeight: 'bold', width: '140px',}}>
                        </TableCell>
                         {desiredFields.map((column) => (
                            <TableCell style={{backgroundColor: 'white', fontWeight: 'bold', width: `${(desiredFields.length > 5 || window.innerWidth < 700) ? '90px' : ''}` }}>
                                 {column}
                            </TableCell>))}
                    </TableRow>
                </TableHead>
                <TableBody>
                    {this.state.items.map((row, index) => {
                        return(
                            <DenseTableViewRow
                                rowName = {row.name}
                                sourceURI={this.state.baseURI}
                                baseURI = {row.uri}
                                index={index}
                                desiredFields={desiredURIs}
                            />)})}
                </TableBody>
            </Table>
        </TableContainer>
        </div>}</>
        );
        
    }

    // TODO: Refactor to not be completely generated by one function
    render() {
        return (
            <>
                {this.generateTable()}
            </>
        );
    }
}
export default withStyles(TableView, STYLES_TABLE_VIEW);
