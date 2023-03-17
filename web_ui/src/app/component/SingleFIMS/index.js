/* eslint-disable camelcase */
import React from 'react';
import io from 'socket.io-client';
import { withStyles } from 'tss-react/mui';
import Grid from '@mui/material/Grid';
import Card from '@mui/material/Card';
import CardContent from '@mui/material/CardContent';
import CardHeader from '@mui/material/CardHeader';
import TextField from '@mui/material/TextField';
import Button from '@mui/material/Button';
import evaluate from 'evaluator.js';
import { STYLES_FEATURES } from '../../styles';
import {
    updatePropertyValue,
} from '../../../AppConfig';
import {
    socket_inspectorControl,
} from '../../../AppAuth';
import FIMSListenHelp from './help';

let theLastListenerDisplayDiv;
const theURIsList = [];
const listenHelperText = `Enter URIs separated by commas to see messages for those URIs (e.g., "/components/bess_aux").
ARRAYS: Put brackets around any query to make it display values only, in neat columns. With an array, you can do calculations like this: "[/assets/ess/ess_1 active_power_setpoint.value, /components/sungrow_ess_1 active_power_setpoint, /site/summary ess_kW_cmd.value] (a+b)/c". Your first query item is represented by "a", your second by "b", etc. You can do any sort of calculation and can use up to ten variables (a through j).
REFERENCES: When using brackets to do calculations on numbers in an array, you can put additional queries before (outside) the bracketed queries. These "references" can then be compared visually to calculated values.
FLAGS: The available flags are "-keys", "-uris", "-exact", and "-csv". They can be used in any order and must be separated from other parts of your query with a space.
KEYS FLAG: Type "-keys" after a URI to see the keys or IDs available at that URI. e.g., "/assets/ess/ess_1 -keys". Once you know the key of the data you want to see, enter a space and the key/ID you want to display, after the URI e.g., "/assets/ess/ess_1 maint_mode". You can enter multiple keys using JavaScript-style dot notation like this: "/assets/ess/ess_1 maint_mode.options.0.name".
URIS FLAG: Type "-uris" after a URI to see the full URIs available at that URI root. e.g., "/assets -uris" or "/assets/ess -uris"
EXACT FLAG: Type "-exact" to view only your exact query, e.g. use "/components/bess_aux -exact" if you don't want to see data from "/components/bess_aux_load".
CSV FLAG: Type "-csv" to output your results in the display area as comma-separated values. These can easily be copy-and-pasted into Excel. In Excel, after pasting, select "Data>Text to Columns..." to separate the CSV into Excel columns. NOTE: Any references, the result of any calculation, and the calculation itself are included in CSV output. Additionally, date, time, and milliseconds columns are added to CSV output for sorting and time reference.
THROTTLE FLAG: Type "-throttle[any number of milliseconds]" to throttle the results. The default is no throttling; the results are displayed as soon as they arrive.`;

const sendInputHelperText = 'Enter a URI and a value to send to it, e.g., /assets/ess/ess_1 \'{ "maint_active_power_setpoint": 0}\'. Multiple URI/value pairs can be sent at once by separating them with a comma,  e.g., /assets/ess/ess_1 \'{ "maint_active_power_setpoint": 0}\', /assets/ess/ess_1 \'{ "maint_reactive_power_setpoint": 0}\'.';
const theQueryValuesArray = [];
const theQueryHeadersArray = [];
let theLastListen = Date.now();

/**
 * Component for FIMS console
 */
class SingleFIMS extends React.PureComponent {
    constructor(props) {
        super(props);
        this.state = {
            theListenerURIs: [],
            showFIMSListenHelp: false,
        };
        this.processListenerInput = this.processListenerInput.bind(this);
        this.turnOffSockets = this.turnOffSockets.bind(this);
        this.checkIfReturn = this.checkIfReturn.bind(this);
        this.clearDisplay = this.clearDisplay.bind(this);
    }

    /**
     * Get session storage information?
     */
    // eslint-disable-next-line class-methods-use-this
    componentDidMount() {
        document.getElementById('sendInput').value = sessionStorage.getItem('sendInput') || '/assets/ess/ess_1 \'{ "maint_active_power_setpoint": 0}\'';
        document.getElementById('listenerInput').value = sessionStorage.getItem('listenerInput') || '/assets/ess/ess_1 active_power_setpoint.value';
        // NOTE: We use socket.io below to communicate with server because it is synchronous. Using
        // updatePropertyValue (basically 'fetch') results in asynchronous results which can be
        // accounted for but is inefficient for quick switching on and off of server functions.
        // socket_inspectorControl.on('message', (data) => {
        //     console.log('socket_inspectorControl')
        //     console.log(data)
        //     if (data.message !== 'SOCKET: JWT verified inspectorControl connection received')
        // console.log('ERROR in socket.io connection to server');
        // });
    }

    /**
     * Set session storage information? and turn off sockets
     */
    componentWillUnmount() {
        sessionStorage.setItem('sendInput', document.getElementById('sendInput').value);
        sessionStorage.setItem('listenerInput', document.getElementById('listenerInput').value);
        this.turnOffSockets();
    }

    /**
     * Lots of calculations and functionality for updating FIMS console
     * @param {*} data 
     * @param {*} theListenerURI 
     * @param {*} theKeys 
     * @param {*} theFlags 
     * @param {*} i 
     * @param {*} theSymbolicCalculation 
     * @param {*} theListenerURIsLength 
     * @param {*} theListenerThrottle 
     */
    updateStateData(data, theListenerURI, theKeys, theFlags, i, theSymbolicCalculation,
        theListenerURIsLength, theListenerThrottle) {
        sessionStorage.setItem('SingleFIMSUpdateStateData', parseInt(sessionStorage.getItem('SingleFIMSUpdateStateData'), 10) + 1);
        const theKeysZero = theKeys[0];
        const listKeys = theFlags.includes('-keys');
        const onlyDisplayExactMatch = theFlags.includes('-exact');
        const displayAsArray = theFlags.includes('-array');
        let theCalculation;
        let theCalculationResult;
        if (this.state.theListenerURIs.length !== 0 && data.uri.includes(theListenerURI)) {
            if (theLastListen + theListenerThrottle < Date.now()) {
                theLastListen = Date.now();
                const theIncomingURI = data.uri;
                if ((onlyDisplayExactMatch && theIncomingURI === theListenerURI)
                    || !onlyDisplayExactMatch) {
                    // eslint-disable-next-line no-param-reassign
                    data = data.body;
                    let theDisplayStringValue = '';
                    let theDisplayStringKey = 'The keys in this object are:';
                    if (listKeys) { // if we just want a list of keys in an object
                        if (theIncomingURI === theListenerURI) {
                            let dataTheKeys = data;
                            theKeys.forEach((key) => {
                                dataTheKeys = dataTheKeys[key];
                            });
                            if (Array.isArray(dataTheKeys)) {
                                // eslint-disable-next-line no-shadow
                                dataTheKeys.forEach((arrayKey, i) => {
                                    theDisplayStringValue += `${i}:${JSON.stringify(arrayKey)}, `;
                                });
                                theDisplayStringKey = 'NOTE: This is an array. The keys along with their objects are:';
                            } else if (typeof dataTheKeys === 'object') {
                                theDisplayStringValue = Object.keys(dataTheKeys);
                            } else {
                                theDisplayStringValue = Object.keys(data);
                            }
                            this.showStringInDisplay(`<strong>${theIncomingURI}</strong> ${theKeysZero || ''} <font color='red'><b>${theDisplayStringKey}</b></font> ${theDisplayStringValue}`, 'unmodified');
                        } else {
                            this.showStringInDisplay(`<strong>${theListenerURI}</strong> <font color='red'><b>There are additional URIs available at this URI root. You can repeat your search with</b></font> ${theListenerURI} uris <font color='red'><b>for a list of the available URIs.</b></font>`, 'unmodified');
                        }
                        // after we get and display the keys, we stop everything
                        this.turnOffSockets();
                    } else if (Object.keys(data).includes(theKeysZero)) {
                        // if something matches the first key then
                        // we're "off to the races"...
                        let dataTheKeys = data;
                        theKeys.forEach((key) => {
                            dataTheKeys = dataTheKeys[key];
                        });
                        if (Array.isArray(dataTheKeys)) {
                            // eslint-disable-next-line no-shadow
                            dataTheKeys.forEach((arrayKey, i) => {
                                theDisplayStringValue += `${i}:${JSON.stringify(arrayKey)}, `;
                            });
                        } else if (typeof dataTheKeys === 'object') {
                            theDisplayStringValue = JSON.stringify(dataTheKeys);
                        } else {
                            theDisplayStringValue = dataTheKeys;
                        }
                        if (theQueryValuesArray[0] !== undefined && theSymbolicCalculation > '') {
                            // theQueryValuesArray[0] may be zero which is falsey, so we have to
                            // make sure it's not undefined instead of simply truthy/falsey
                            let theReferencesString = '';
                            if (theQueryValuesArray.length > theListenerURIsLength) {
                                for (let n = theListenerURIsLength; n < theQueryValuesArray.length;
                                    n += 1) {
                                    theReferencesString += (theQueryValuesArray[n] !== undefined && typeof theQueryValuesArray[n] !== 'string') ? theQueryValuesArray[n].toFixed(10).toString().padStart(22, 'æ') : theQueryValuesArray[n];
                                }
                                theReferencesString = `<b>References:</b>&nbsp;&nbsp;${theReferencesString.replace(/æ/g, '&nbsp;')}`;
                            }
                            try {
                                theCalculation = theSymbolicCalculation.replace(/a/g, theQueryValuesArray[0])
                                    .replace(/b/g, theQueryValuesArray[1] || '')
                                    .replace(/c/g, theQueryValuesArray[2] || '')
                                    .replace(/d/g, theQueryValuesArray[3] || '')
                                    .replace(/e/g, theQueryValuesArray[4] || '')
                                    .replace(/f/g, theQueryValuesArray[5] || '')
                                    .replace(/g/g, theQueryValuesArray[6] || '')
                                    .replace(/h/g, theQueryValuesArray[7] || '')
                                    .replace(/i/g, theQueryValuesArray[8] || '')
                                    .replace(/j/g, theQueryValuesArray[9] || '');
                                // then, if the number of alpha characters in the symbolic
                                // calculation equals the length of the query values array
                                // we'll show the result
                                if (theSymbolicCalculation.replace(/[^a-z]/gi, '').replace(/[A-Z]/g, '').length === theQueryValuesArray.length) {
                                    theCalculationResult = `<b>Calculation:</b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;${theCalculation}<br><b>Result:</b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;${evaluate(theCalculation).toString().padStart(22, 'æ').replace(/æ/g, '&nbsp;')}<br>${theReferencesString}`;
                                }
                            } catch (err) { console.error(err); }
                        }
                        theQueryValuesArray[i] = theDisplayStringValue;
                        // this inserts the variable values
                        if (theFlags.includes('-csv')) theQueryHeadersArray[i] = `${theListenerURI} ${theKeys.join('.')}`;
                        // into an array in order, as they come in.
                        let theQueryValuesString = '';
                        if (displayAsArray) {
                            if (theFlags.includes('-csv')) {
                                if ((theSymbolicCalculation && theCalculation)
                                    || !theSymbolicCalculation) {
                                    try {
                                        theQueryValuesString = `${theQueryValuesArray.join(',')},${evaluate(theCalculation)},=${theCalculation},${new Date().toLocaleString()},${Date.now()}`;
                                    } catch (err) {
                                        theQueryValuesString = `<i>waiting for values...</i> ${theQueryValuesArray.join(',')}`;
                                        // evaluate.js will sometimes produce "insufficient operands
                                        // for operator" error on the first iteration. It can be
                                        // ignored.
                                    }
                                }
                            } else if (theCalculationResult) {
                                // Once we've got all the values in our array then we should
                                // get a valid result and end up here. Otherwise, we'll just
                                // keep displaying the values, padded-out below.
                                theQueryValuesString = theCalculationResult;
                            } else {
                                // the character we padStart with is (on Mac) [option]'
                                // (option-single quote) we use this because we need a *single*
                                // character which isn't going to naturally appear in our data
                                // that we will substitute with &nbsp; later. Using &nbsp; in
                                // the padStart() does not work, we must do this substitution.
                                theQueryValuesArray.forEach((value, n) => {
                                    if (typeof value === 'string' || typeof value === 'boolean') {
                                        theQueryValuesString += `&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;${value}`;
                                        // we use i === n below so we don't show the keys from other
                                        // bracketed queries
                                        if (i === n && theSymbolicCalculation) this.showStringInDisplay(`ERROR: The query result for '${theListenerURI} ${theKeys.join('.')}' is a string. Calculations can only be performed on numeric values.`, 'alert');
                                    } else {
                                        theQueryValuesString += value.toFixed(10).toString().padStart(22, 'æ');
                                    }
                                });
                                theQueryValuesString = `<b>Values:</b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;${theQueryValuesString.replace(/æ/g, '&nbsp;')}`;
                                if ((i + 1 > theListenerURIsLength) && !theSymbolicCalculation) this.showStringInDisplay('NOTE: Your query includes a reference (the part before the \'[]\' brackets) but no calculation (a part after the brackets). Be aware that the display of your reference(s) will appear *after* the bracketed item(s).', 'alert');
                            }
                        }
                        this.showStringInDisplay(displayAsArray ? `${theQueryValuesString}` : `<strong>${theIncomingURI}</strong> ${theKeys.join('.')}: ${theDisplayStringValue}`, 'unmodified');
                    } else { // if no key is matched or there is no key
                        // eslint-disable-next-line no-lonely-if
                        if (theKeysZero) { // there is a key but it didn't match anything
                            this.showStringInDisplay(`<strong>${theIncomingURI}</strong> <font color='red'><b>KEY "${theKeysZero}" NOT FOUND. Displaying complete object.</b></font>${JSON.stringify(data)}`, 'unmodified');
                        } else { // there is no key, just a URI
                            this.showStringInDisplay(`<strong>${theIncomingURI}</strong> ${JSON.stringify(data)}`, 'unmodified');
                        }
                    }
                }
            }
        }
    }

    /**
     * Checks for return key event
     * @param {*} event
     */
    checkIfReturn(event) { // executes when user presses [return] in TextField
        if (event.which === 13 || event.keyCode === 13) {
            if (event.target.id === 'listenerInput') this.processListenerInput();
            if (event.target.id === 'sendInput') {
                this.sendFIMS();
            }
        }
    }

    /**
     * Formatting for different strings in display
     * @param {string} string
     * @param {string} type [alert, bold, unmodified]
     */
    // eslint-disable-next-line class-methods-use-this
    showStringInDisplay(string, type) {
        const newData = document.createElement('div');
        if (theLastListenerDisplayDiv) {
            newData.classList.add('alternating-background-color');
        }
        theLastListenerDisplayDiv = !theLastListenerDisplayDiv;
        const listenerDisplay = document.getElementById('listenerDisplay');
        switch (type) {
            case 'alert':
                newData.innerHTML = `<font color='red'><b>${string}</b></font>`;
                break;
            case 'bold':
                newData.innerHTML = `<b>${string}</b>`;
                break;
            case 'unmodified':
                newData.innerHTML = string;
                break;
            default:
                newData.innerHTML = string;
        }
        listenerDisplay.insertBefore(newData, listenerDisplay.firstChild);
    }

    /**
     * Processes listener inputs
     */
    processListenerInput() {
        this.turnOffSockets();
        const theListenerURIs = [];
        let theFlags = [];
        let theReferences = [];
        let theSymbolicCalculation = '';
        let theListenerThrottle = 0;
        socket_inspectorControl.emit('message', { sender: 'SingleFIMS', message: 'start listening' });
        try {
            let theFullQuery = document.getElementById('listenerInput').value;
            if (theFullQuery.includes('-throttle')) {
                theListenerThrottle = parseInt(theFullQuery.match(/ -throttle\d*/g, '')[0].trim().replace('-throttle', ''), 10);
                theFullQuery = theFullQuery.replace(/ -throttle\d*/g, '');
            }
            if (theFullQuery.includes('-csv')) {
                theFlags.push('-csv');
                theFullQuery = theFullQuery.replace(' -csv', '');
            }
            if (theFullQuery.includes('[') && theFullQuery.includes(']')) {
                theFlags.push('-array');
                // if there are references then we'll get a value here
                if (theFullQuery.includes(' [')) theReferences = theFullQuery.split(' [')[0].split(',');
                /* eslint-disable prefer-destructuring */
                theSymbolicCalculation = theFullQuery.split('] ')[1];
                theFullQuery = theFullQuery.split('] ')[0];
                // if there are references then we need to drop them out:
                if (theFullQuery.includes(' [')) theFullQuery = theFullQuery.split(' [')[1];
                /* eslint-enable prefer-destructuring */
                theFullQuery = theFullQuery.replace('[', '');
                theFullQuery = theFullQuery.replace(']', '');
            }
            const theStringsEntered = theFullQuery.split(',');
            if (theReferences) {
                theReferences.forEach((reference) => {
                    theStringsEntered.push(reference);
                });
            }
            if (theStringsEntered.length > 1) {
                theStringsEntered.forEach((string) => {
                    if (string.includes('-keys')) {
                        this.showStringInDisplay('NOTE: Because the \'-keys\' flag retrieves a set of data and then stops, it can cause problems when used as part of multiple comma-separated queries. If you are not getting the info you want, please enter your keys query as a single query, separate from others.', 'alert');
                    }
                });
            }
            theStringsEntered.forEach((string, i) => {
                // eslint-disable-next-line no-param-reassign
                string = string.trim();
                if (string.includes('components')) {
                    socket_inspectorControl.emit('message', { sender: 'SingleFIMS', message: 'open components' });
                }
                const stringSplitZero = string.split(' ')[0];
                const theURI = stringSplitZero.charAt(0) === '/' ? stringSplitZero.slice(1, stringSplitZero.length) : stringSplitZero;
                const theArguments = string.split(' ').splice(1, string.split(' ').length);
                theFlags = theFlags.concat(theArguments.filter((arg) => arg.startsWith('-')));
                // the following should just be a single string in dot notation
                // like 'maint_mode.options'
                let theKeys = theArguments.filter((arg) => !arg.startsWith('-'));
                if (theKeys.length > 1) {
                    // this would be caused by using spaces instead of dot notation
                    this.showStringInDisplay('ERROR: Please use dot notation (e.g. \'maint_mode.options\' instead of \'maint_mode options\'', 'alert');
                    return;
                }
                theKeys = theKeys.toString().split('.');
                const theListenerURI = `/${theURI}`;
                const theSocket = io(`/${theURI.split('/')[0]}`, { secure: true });
                theListenerURIs.push(theSocket);
                if (theFlags.includes('-uris')) { // then we list the URIs
                    theURIsList.length = 0;
                    theSocket.on('message', (data) => {
                        const theData = JSON.parse(data);
                        if (theData.uri.includes(theListenerURI)
                            && !theURIsList.includes(theData.uri)) {
                            theURIsList.push(theData.uri);
                        }
                    });
                    setTimeout(() => {
                        this.turnOffSockets();
                        this.showStringInDisplay(`<strong>${theListenerURI}</strong> <font color='red'><b>The available URIs for this URI root are:</b></font> ${theURIsList.sort()}`, 'unmodified');
                    }, 2100);
                    this.showStringInDisplay(`<strong>${theListenerURI}</strong> Collecting available URIs...`, 'unmodified');
                } else {
                    theSocket.on('message', (data) => this.updateStateData(JSON.parse(data),
                        theListenerURI, theKeys, theFlags, i, theSymbolicCalculation,
                        (theListenerURIs.length - theReferences.length), theListenerThrottle));
                }
            });
        } catch (err) {
            console.log('processListenerInput: could not process:', err);
        }
        this.setState({
            theListenerURIs,
        });
        document.getElementById('listening').focus();
    }

    /**
     * Turns off sockets
     */
    turnOffSockets() {
        // tell the server
        socket_inspectorControl.emit('message', { sender: 'SingleFIMS', message: 'stop listening' });
        this.state.theListenerURIs.forEach((socket) => {
            socket.disconnect();
        });
        this.setState({
            theListenerURIs: [],
        });
        // empty theQueryValuesArray
        theQueryValuesArray.length = 0;
        // display the headers in the display if we're outputting -csv
        if (theQueryHeadersArray.length > 0) this.showStringInDisplay(`${theQueryHeadersArray.join(',')},result,calculation,date, time, milliseconds`);
        theQueryHeadersArray.length = 0;
    }

    /**
     * Empties display field
     */
    clearDisplay() {
        this.setState({
            data: '',
        });
    }

    /**
     * Sends a FIMS message with state info
     */
    // eslint-disable-next-line class-methods-use-this
    sendFIMS() {
        document.getElementById('sendInput').classList.add('flash-green');
        setTimeout(() => {
            document.getElementById('sendInput').classList.remove('flash-green');
        }, 1000);
        const theURIsEntered = document.getElementById('sendInput').value.split(',');
        theURIsEntered.forEach((message) => {
            const theURIEntered = message.trim().split(' ')[0];
            const theBodyEntered = JSON.parse(message.split('\'')[1]);
            let api_path;
            let api_endpoint;
            let value_to_send;
            Object.keys(theBodyEntered).forEach((key) => {
                api_path = theURIEntered;
                if (api_path.charAt(0) === '/') api_path = api_path.slice(1, api_path.length);
                api_endpoint = key;
                value_to_send = theBodyEntered[key];
                console.log(`${api_path} '{"${api_endpoint}": ${value_to_send}}'`);
                updatePropertyValue(api_path, api_endpoint, value_to_send)
                    .then((response) => {
                        if (response.ok) {
                            return response.json();
                        }
                        throw new Error(`${response.statusText} : Cannot updatePropertyValue() for: ${response.url}`);
                    })
                    .catch((error) => {
                        throw new Error(`SingleFIMS/sendFIMS error: ${error}`);
                    });
            });
        });
    }

    /**
     * Displays full page help
     */
    openFullPageHelp = () => {
        this.setState({ showFIMSListenHelp: true });
    }

    /**
     * Closes full page help
     */
    closeFullPageHelp = () => {
        this.setState({ showFIMSListenHelp: false });
    }

    /**
     * Retrieves state information from child components
     * @param {*} event child event
     */
    getStateFromChild = (focusedFieldID) => {
        this.setState({
            focusedFieldID,
        });
    };

    render() {
        sessionStorage.setItem('SingleFIMSRender', parseInt(sessionStorage.getItem('SingleFIMSRender'), 10) + 1);
        const listening = this.state.theListenerURIs.length > 0;
        setTimeout(() => {
            if (!listening && document.getElementById('listenerInput')) document.getElementById('listenerInput').focus();
        }, 200);
        const buttonColor = listening ? 'secondary' : 'primary';
        const buttonName = listening ? 'Stop Listening' : 'Listen';
        const buttonOnClick = listening ? this.turnOffSockets : this.processListenerInput;
        return (
            <>
                {this.state.showFIMSListenHelp ? <FIMSListenHelp onClick={this.closeFullPageHelp} /> : ''}
                <Grid item md={12}>
                    <Card>
                        <CardHeader
                            title='FIMS Send and Listen'
                        />
                        <CardContent style={{ textAlign: 'center' }}>
                            <div style={{ textAlign: 'right' }}>
                                <Button
                                    style={{ margin: 8 }}
                                    id='help-button'
                                    variant='contained'
                                    color='primary' // this is the bg color of the button
                                    onClick={this.openFullPageHelp}
                                >
                                    {'Help'}
                                </Button>
                            </div>
                            <TextField
                                id='sendInput'
                                onKeyUp={this.checkIfReturn}
                                variant='outlined'
                                label=''
                                helperText={sendInputHelperText}
                            />
                            <Button
                                style={{ margin: 8 }}
                                id='sendFIMS'
                                variant='contained'
                                color='primary' // this is the bg color of the button
                                onClick={this.sendFIMS}
                            >
                                {'Send FIMS'}
                            </Button>
                            <br />
                            <hr style={{ width: '70%', border: '2px solid #dedede' }} />
                            <br />
                            <TextField
                                id='listenerInput'
                                onKeyUp={this.checkIfReturn}
                                variant='outlined'
                                label=''
                                helperText={listenHelperText}
                            />
                            <Button
                                style={{ margin: 8 }}
                                id='listening'
                                variant='contained'
                                color={buttonColor} // this is the bg color of the button
                                onClick={buttonOnClick}
                            >
                                {buttonName}
                            </Button>
                            <Button
                                style={{ margin: 8 }}
                                variant='contained'
                                color='primary' // this is the bg color of the button
                                onClick={this.clearDisplay}
                            >
                                {'Clear Display'}
                            </Button>
                            <br />
                            <br />
                            <div id='listenerDisplay' overflow='auto' style={{
                                fontFamily: '"Courier New", Courier, monospace', fontSize: '0.8em', width: '100%', height: '65vh', textAlign: 'left',
                            }} note='this div requires inline styling, CSS styling does not work inside CardContent'>{this.state.data}</div>
                        </CardContent>
                    </Card>
                </Grid>
            </>
        );
    }
}
export default withStyles(SingleFIMS, STYLES_FEATURES);
