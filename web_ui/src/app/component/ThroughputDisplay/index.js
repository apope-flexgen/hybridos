import React from "react";
import Radio from '@mui/material/Radio';
import RadioGroup from '@mui/material/RadioGroup';
import FormControlLabel from '@mui/material/FormControlLabel';
import FormControl from '@mui/material/FormControl';
import FormLabel from '@mui/material/FormLabel';
import Card from '@mui/material/Card';
import CardContent from "@mui/material/CardContent";
import Typography from "@mui/material/Typography";
import Table from '@mui/material/Table'
import TableContainer from "@mui/material/TableContainer";
import TableRow from "@mui/material/TableRow";
import TableCell from '@mui/material/TableCell'
import { withStyles } from 'tss-react/mui';

import {EVENTS_PAGE_PATH} from '../../../AppConfig';
import {
    socket_heart1000,
    socket_serverCount,
} from '../../../AppAuth';
import { RowingSharp } from "@mui/icons-material";
import { STYLES_THROUGHPUT_DISPLAY } from "../../styles";

let timestampError = 0;
let timestampErrorString = '';

class ThroughputDisplay extends React.PureComponent {
    constructor(props) {
        super(props);
        this.state = {
            name: 'Throughput Display',
            value: null,
            defaultValue: 'off',
            throughput_display: 'off',
            options:
                [
                    { name: 'Footer', value: 'footer' },
                    { name: 'Console', value: 'console' },
                    { name: 'Off', value: 'off' },
                ],
            countUpdateOutput: '',
            updatesArr: [],
            rendersArr: [],
            emitURIArr: [],
            FIMSstr: null,
        };
    }

    countUpdates = (theCountData) => {
        if (document.getElementById('inspector') !== null) {
            try {
                const theTelemetrics = ['SummaryCard', 'SingleSite', 'SingleFeature', 'SingleAsset', 'EnhancedTable', 'AssetsPage', 'ComponentsPage', 'Features', 'Site', 'SingleFIMS'];
                let theUpdatesDisplayString = 'Updates:  ';
                let theRendersDisplayString = 'Renders:  ';
                let updatesArr = [];
                let rendersArr = [];
                theTelemetrics.forEach((page) => {
                    let theNumberTemp = sessionStorage.getItem(`${page}UpdateStateData`) || 0;
                    theUpdatesDisplayString += `${theNumberTemp.toString().padStart(3, ' ')} | `;
                    updatesArr.push(theNumberTemp);
                    theNumberTemp = sessionStorage.getItem(`${page}Render`) || 0;
                    theRendersDisplayString += `${theNumberTemp.toString().padStart(3, ' ')} | `;
                    rendersArr.push(theNumberTemp);
                });
                let {pubRateSite, pubRateFeatures, pubRateAssets, pubRateComponents, pubRateInspector,
                    setRateSite, setRateFeatures, setRateAssets, setRateComponents, setRateInspector, setRateEvents,
                    getRateSite, getRateFeatures, getRateAssets, getRateComponents, getRateInspector, getRateEvents,
                    postRateSite, postRateFeatures, postRateAssets, postRateComponents, postRateInspector, postRateEvents,
                    theListenForMessagesCount, theReceivedMessagesCount, theMaxReceivedMessagesCount, count} = theCountData;
                let theCountString = `EMIT URI  site  feat  asst  comp  evnt  insp
Pubs:     ${pubRateSite.toString().padStart(3, ' ')} | ${pubRateFeatures.toString().padStart(3, ' ')} | ${pubRateAssets.toString().padStart(3, ' ')} | ${pubRateComponents.toString().padStart(3, ' ')} |  -  | ${pubRateInspector.toString().padStart(3, ' ')}
Sets:     ${setRateSite.toString().padStart(3, ' ')} | ${setRateFeatures.toString().padStart(3, ' ')} | ${setRateAssets.toString().padStart(3, ' ')} | ${setRateComponents.toString().padStart(3, ' ')} | ${setRateEvents.toString().padStart(3, ' ')} | ${setRateInspector.toString().padStart(3, ' ')}
Gets:     ${getRateSite.toString().padStart(3, ' ')} | ${getRateFeatures.toString().padStart(3, ' ')} | ${getRateAssets.toString().padStart(3, ' ')} | ${getRateComponents.toString().padStart(3, ' ')} | ${getRateEvents.toString().padStart(3, ' ')} | ${getRateInspector.toString().padStart(3, ' ')}
Posts:    ${postRateSite.toString().padStart(3, ' ')} | ${postRateFeatures.toString().padStart(3, ' ')} | ${postRateAssets.toString().padStart(3, ' ')} | ${postRateComponents.toString().padStart(3, ' ')} | ${postRateEvents.toString().padStart(3, ' ')} | ${postRateInspector.toString().padStart(3, ' ')}

FIMS listens: ${theListenForMessagesCount} / FIMS receives: ${theReceivedMessagesCount} / Max FIMS receives: ${theMaxReceivedMessagesCount}
Heartbeat: ${count} / Uptime: ${(count / 3600).toFixed(2)} hours`;

                const countUpdateOutputTemp = `
Updates and FIMS messages per second
------------------------------------
PAGE      Summ  SnSt  SnFt  SnAs  EnTb  AsPg  CmPg  Feat  Site  SnFM  SnCh
${theUpdatesDisplayString.slice(0, -2)}
${theRendersDisplayString.slice(0, -2)}
    
${theCountString} ${timestampErrorString}`;
                if (this.state.throughput_display === 'console') {
                    console.log(countUpdateOutputTemp);
                }
                if (this.state.throughput_display === 'footer') {
                    // the .replace() below changes line feeds to <br> for HTML formatting
                    // in the footer.
                    let emitURIArr = [['Pubs: ', pubRateSite, pubRateFeatures, pubRateAssets, pubRateComponents, '-', pubRateInspector],
                                        ['Sets: ', setRateSite, setRateFeatures, setRateAssets, setRateComponents, setRateEvents, setRateInspector],
                                        ['Gets: ', getRateSite, getRateFeatures, getRateAssets, getRateComponents, getRateEvents, getRateInspector],
                                        ['Posts: ', postRateSite, postRateFeatures, postRateAssets, postRateComponents, postRateEvents, postRateInspector]];
                    let FIMSstr = `FIMS listens: ${theListenForMessagesCount} / FIMS receives: ${theReceivedMessagesCount} / Max FIMS receives: ${theMaxReceivedMessagesCount}
                                   Heartbeat: ${count} / Uptime: ${(count / 3600).toFixed(2)} hours`;
                    this.setState({ countUpdateOutput: countUpdateOutputTemp.replace(/(?:\r\n|\r|\n)/g, '<br>').replace(/ /g, '\u00a0'),
                                    updatesArr, rendersArr, emitURIArr, FIMSstr });
                }

            } catch (err) { console.error(err); }
            sessionStorage.setItem('SummaryCardUpdateStateData', 0);
            sessionStorage.setItem('SummaryCardRender', 0);
            sessionStorage.setItem('SingleSiteUpdateStateData', 0);
            sessionStorage.setItem('SingleSiteRender', 0);
            sessionStorage.setItem('SingleFeatureUpdateStateData', 0);
            sessionStorage.setItem('SingleFeatureRender', 0);
            sessionStorage.setItem('SingleAssetUpdateStateData', 0);
            sessionStorage.setItem('SingleAssetRender', 0);
            sessionStorage.setItem('SingleFIMSUpdateStateData', 0);
            sessionStorage.setItem('SingleFIMSRender', 0);
            sessionStorage.setItem('EnhancedTableUpdateStateData', 0);
            sessionStorage.setItem('EnhancedTableRender', 0);
            sessionStorage.setItem('AssetsPageUpdateStateData', 0);
            sessionStorage.setItem('AssetsPageRender', 0);
            sessionStorage.setItem('ComponentsPageUpdateStateData', 0);
            sessionStorage.setItem('ComponentsPageRender', 0);
            sessionStorage.setItem('FeaturesUpdateStateData', 0);
            sessionStorage.setItem('FeaturesRender', 0);
            sessionStorage.setItem('SiteUpdateStateData', 0);
            sessionStorage.setItem('SiteRender', 0);
        }
    }

    getStateFromChild = (event) => {
        let eventTargetValue = event.target.value;
        // the following block is a required kludge. For some reason, switching
        // directly from 'console' to 'footer' or vice-versa, stops the console
        // or footer from being updated. Switching to 'off' in between solves the
        // problem. So here we check to see if it's going from 'console' to
        // 'footer' or 'footer' to 'console' and, if so, we set state to off and
        // then right back to whatever it is supposed to be. I believe this
        // problem is caused by some obscure React bug or quirk. I have found no
        // other way to get around this problem. -DM 031620
        if (!(eventTargetValue === 'off' || this.state.throughput_display === 'off') && eventTargetValue !== this.state.throughput_display) {
            const thoughputDisplayValueTemp = eventTargetValue;
            eventTargetValue = 'off';
            this.setState({
                throughput_display: 'off',
            }, () => {
                this.setState({
                    throughput_display: thoughputDisplayValueTemp,
                });
            });
        }

        switch (eventTargetValue) {
            case 'off':
                break;
            default:
                socket_serverCount.on('/serverCount', (theCountData) => {
                    // console.log('serverCount received')
                    this.countUpdates(theCountData);
                });
                socket_heart1000.on('/heart1000', (msg) => {
                    const theNewTimestamp = (JSON.parse(msg).timestamp).toString().slice(-3);
                    timestampErrorString = `/ The deviation in milliseconds for beat ${JSON.parse(msg).count} was ${(theNewTimestamp - timestampError).toString().padStart(2, ' ')} or ${((Math.abs(theNewTimestamp - timestampError)) / 100).toFixed(2)}%`;
                    timestampError = theNewTimestamp;
                });
        }
        this.setState({
            [event.target.name]: event.target.value,
        }, this.turnOffSockets);
    };

    /**
     * Turns off sockets
     */
    turnOffSockets = () => {
        if (this.state.throughput_display !== 'footer' && this.state.throughput_display !== 'console') {
            socket_heart1000.off('/heart1000');
            socket_serverCount.off('/serverCount');
        }
    }


    render() {
        const {pathname} = this.props;
        let {updatesArr, rendersArr, emitURIArr, FIMSstr} = this.state;
        FIMSstr = `${FIMSstr} ${timestampErrorString}`;

        return (
            <React.Fragment>
                <FormControl
                    style={{
                        width: '12em', padding: 10, backgroundColor: '#eee' }}
                >
                <FormLabel style={{ color: 'black' }}>{this.state.name}</FormLabel>
                    <RadioGroup
                        style={{ marginTop: 10, }}
                        aria-label={this.props.id}
                        name={this.state.name.toLowerCase().replace(/ /g, '_')}
                        value={this.state.throughput_display || ''}
                        onClick={this.getStateFromChild} >
                        {this.state.options.map((element) => (
                            <FormControlLabel
                                style={{ margin: -10 }}
                                key={element.name.toLowerCase()}
                                value={element.value}
                                control={<Radio color='primary' />}
                                label={element.name}
                                data-cy={element.name.toLowerCase().replace(/ /g, '_')}
                            />
                        ))}
                    </RadioGroup>
                </FormControl>
            {pathname !== EVENTS_PAGE_PATH && this.state.throughput_display === 'footer' &&
                <Card
                    style={{ backgroundColor: 'rgb(237, 243, 244)', position: 'fixed', bottom: '2px', left: '250px', zIndex: 999, height: 390, width: 1055, paddingLeft: 20, paddingRight: 20 }}>
                        <CardContent>
                            <Typography variant="h6">
                                Updates and FIMS messages per second
                            </Typography>
                        </CardContent>
                        <TableContainer>
                            <Table size='small' component='div'>
                                <TableRow style={{ backgroundColor: 'rgba(0, 220, 249, 0.35)' }}>
                                    <TableCell style={{ fontWeight: 'bold' }}>PAGE</TableCell>
                                    <TableCell style={{ fontWeight: 'bold' }}>Summ</TableCell>
                                    <TableCell style={{ fontWeight: 'bold' }}>SnSt</TableCell>
                                    <TableCell style={{ fontWeight: 'bold' }}>SnFt</TableCell>
                                    <TableCell style={{ fontWeight: 'bold' }}>SnAs</TableCell>
                                    <TableCell style={{ fontWeight: 'bold' }}>EnTb</TableCell>
                                    <TableCell style={{ fontWeight: 'bold' }}>AsPg</TableCell>
                                    <TableCell style={{ fontWeight: 'bold' }}>CmPg</TableCell>
                                    <TableCell style={{ fontWeight: 'bold' }}>Feat</TableCell>
                                    <TableCell style={{ fontWeight: 'bold' }}>Site</TableCell>
                                    <TableCell style={{ fontWeight: 'bold' }}>SnFM</TableCell>
                                    <TableCell style={{ fontWeight: 'bold' }}>SnCh</TableCell>
                                </TableRow>
                                <TableRow style={{ height: 20}}>
                                    <TableCell>Updates: </TableCell>
                                    {updatesArr.map((row) => (
                                        <TableCell style={{width: 50}}>{row}</TableCell>
                                    ))
                                    }
                                </TableRow>
                                <TableRow>
                                    <TableCell>Renders: </TableCell>
                                    {rendersArr.map((row) => (
                                        <TableCell>{row}</TableCell>
                                    ))
                                    }
                                </TableRow>
                            </Table>
                        </TableContainer>

                        <TableContainer>
                            <Table size='small'>
                                <TableRow style={{ backgroundColor: 'rgba(0, 220, 249, 0.35)', fontWeight: 'bold' }}>
                                    <TableCell style={{ fontWeight: 'bold' }}>EMIT URI</TableCell>
                                    <TableCell style={{ fontWeight: 'bold' }}>site</TableCell>
                                    <TableCell style={{ fontWeight: 'bold' }}>feat</TableCell>
                                    <TableCell style={{ fontWeight: 'bold' }}>asst</TableCell>
                                    <TableCell style={{ fontWeight: 'bold' }}>comp</TableCell>
                                    <TableCell style={{ fontWeight: 'bold' }}>evnt</TableCell>
                                    <TableCell style={{ fontWeight: 'bold' }}>insp</TableCell>
                                </TableRow>
                                {emitURIArr.map((row) => (
                                    <TableRow>
                                        {row.map((cell) => (
                                            <TableCell>{cell}</TableCell>
                                        ))}
                                    </TableRow>
                                ))}
                            </Table>
                        </TableContainer>
                        <CardContent>
                            <Typography variant="body2" display="block" style={{wordWrap: 'break-word'}}>
                                {FIMSstr}
                            </Typography>
                        </CardContent>
                </Card>
            }
            </React.Fragment>
        );
    }
}
//dangerouslySetInnerHTML={{ __html: this.state.countUpdateOutput}}
export default withStyles(ThroughputDisplay, STYLES_THROUGHPUT_DISPLAY);