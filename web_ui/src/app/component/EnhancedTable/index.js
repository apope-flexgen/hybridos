/* eslint-disable no-unused-expressions */
/* eslint-disable no-underscore-dangle */
/* eslint-disable camelcase */
import React from 'react';
import PropTypes from 'prop-types';
import { withStyles } from 'tss-react/mui';
import LoadingHOC from '../LoadingHOC';
import FormControlLabel from '@mui/material/FormControlLabel';
import { Checkbox } from '@mui/material';
import { STYLES_EVENTS } from '../../styles';
import DatePicker from 'react-date-picker';
import {
    isLoading,
    getDataForURI,
} from '../../../AppConfig';
import {
    socket_events,
    siteConfiguration,
} from '../../../AppAuth';

import ActivityIndicator from '../ActivityIndicator';
import { Button, InputLabel, MenuItem, FormControl, Select, TextField } from '@mui/material';
import classNames from 'classnames';

const moment = require('moment');
require('moment-timezone');

/**
 * Material UI theme properties
 * @param {*} theme
 */
const styles = (theme) => ({
    root: {
        width: '100%',
        marginTop: theme.spacing(),
    },
    table: {},
    tableWrapper: {
        overflowX: 'auto',
    },
    default: {
        backgroundColor: 'none',
    },
    info: {
        backgroundColor: 'rgb(221, 255, 221)',
    },
    status: {
        backgroundColor: 'rgb(221, 255, 221)',
    },
    alarm: {
        backgroundColor: 'gold',
    },
    fault: {
        backgroundColor: 'rgb(255, 221, 221)',
    },
});

/**
 * Puts data into an object?
 * @param {*} id id
 * @param {*} source source
 * @param {*} message message
 * @param {*} severity severity
 * @param {*} timestamp timestamp
 */
function createData(id, source, message, severity, timestamp) {
    return {
        id, source, message, severity, timestamp,
    };
}

/**
 * Component for rendering an enhanced table
 */
class EnhancedTable extends React.PureComponent {
    constructor(props) {
        super(props);
        this.state = {
            eventsQueryID: null,
            order: 'desc',
            orderBy: 'timestamp',
            selected: [],
            data: [],
            page: 0,
            rowsPerPage: 12,
            eventSeverityValue: [1, 2, 3, 4],
            eventDateRangeAfter: '',
            eventDateRangeBefore: '',
            eventSortValue: { timestamp: -1 },
            eventPageValue: '1',
            totalNumberInQuery: 12,
            intervalId: null,
            api_endpoint: 'events',
            socket_listener_uri: '/events',
            textBoxText: '',
            numbersLoaded: true,
            eventsLoaded: true,
            eventsLoading: false,
            initialDateAfter: '',
            initialDateBefore: '',
        };
        this.props.setLoading(true)
        this._isMounted = false;
        this.handleSelect = this.handleSelect.bind(this)
        this.handleSearch = this.handleSearch.bind(this)
        this.handleTextBox = this.handleTextBox.bind(this)
    }

    // eslint-disable-next-line class-methods-use-this
    /**
     * Connect socket
     */
    UNSAFE_componentWillMount() {
        socket_events.on(this.state.socket_listener_uri, (data) => {
            this.updateStateData(JSON.parse(data));
        });
    }

    /**
     * Start socket fetch loop
     */
    componentDidMount() {
        this._isMounted = true;
        this._isMounted && this.setState({ timezone: siteConfiguration.timezone });
        setTimeout(() => {
            this._isMounted && this.fetchEvents();
        }, 1000);
    }

    /**
     * Unmount and disconnect socket
     */
    componentWillUnmount() {
        socket_events.off(this.state.socket_listener_uri);
        this._isMounted = false;
    }

    /**
     * Fetch events from socket
     */
    fetchEvents() {
        this.props.setLoading(true)
        const monthAgo = new Date(new Date() - 2592000000)
        const monthAgoString = monthAgo.toISOString().split('T')[0];
        const dateAfter = this.state.eventDateRangeAfter ? this.state.eventDateRangeAfter.toISOString().split('T')[0] : monthAgoString;
        const dateBeforeTemp = new Date(); // today
        dateBeforeTemp.setDate(new Date().getDate() + 1); // tomorrow
        // eslint-disable-next-line no-new
        new Date(Number(dateBeforeTemp)); // tomorrow coerced
        const dateBefore = this.state.eventDateRangeBefore ? this.state.eventDateRangeBefore.toISOString().split('T')[0] : dateBeforeTemp.toISOString().split('T')[0];
        this.setState({ initialDateAfter: dateAfter, initialDateBefore: dateBefore})
        // this first query gets the total count of records, the second
        // query gets the chunk of 12 records to display
        // we create a random eventsQueryID so we can filter our results from others' later
        const eventsQueryID = parseInt(`${Date.now()}${(Math.floor(100000 + Math.random() * 900000))}`, 10);
        this.setState({ eventsQueryID });
        // assume time from user is in timezone from web_ui.json,
        // then convert it to UTC (Zulu) time for fims (previously theBody formatted to Zulu)
        let theBody = encodeURIComponent(`{"eventsQueryID": "${eventsQueryID}",` + 
            `"severity":[${(this.state.eventSeverityValue).toString()}],` +
            `"message":"${this.state.textBoxText}",` +
            `"after":"${moment.tz(dateAfter, this.state.timezone).utc().format('YYYY-MM-DDTHH:mm:ss')}.000",` +
            `"before":"${moment.tz(dateBefore, this.state.timezone).subtract(1, 'seconds').utc().format('YYYY-MM-DDTHH:mm:ss')}.000",` +
            `"count":true}`);
        let api_endpoint = `events/${theBody}`;
        getDataForURI(api_endpoint)
            .then((response) => {
                if (response.ok) {
                    return response.json();
                }
                throw new Error(`${response.statusText} : Cannot get events for: ${response.url}`);
            })
            .catch((error) => this.setState({ error }));
        theBody = encodeURIComponent(`{"eventsQueryID": "${eventsQueryID}",` +
            `"severity":[${(this.state.eventSeverityValue).toString()}],` +
            `"message":"${this.state.textBoxText}",` +
            `"after":"${moment.tz(dateAfter, this.state.timezone).utc().format('YYYY-MM-DDTHH:mm:ss')}.000",` +
            `"before":"${moment.tz(dateBefore, this.state.timezone).subtract(1, 'seconds').utc().format('YYYY-MM-DDTHH:mm:ss')}.000",` +
            `"sort":${JSON.stringify(this.state.eventSortValue)},` +
            `"limit":12,` +
            `"page":${this.state.eventPageValue}}`);
        api_endpoint = `events/${theBody}`;
        getDataForURI(api_endpoint)
            .then((response) => {
                if (response.ok) {
                    return response.json();
                }
                throw new Error(`${response.statusText} : Cannot get events for: ${response.url}`);
            })
            .catch((error) => this.setState({ error }));
    }

    /**
     * Update state with data from fetch
     * @param {*} data
     */
    updateStateData(data) {
        if (data[0].eventsQueryID > 0) {
            // if the first element is an eventsQueryID
            const eventsQueryID = data.shift();
            if (parseInt(eventsQueryID.eventsQueryID, 10) === this.state.eventsQueryID) {
                // if the eventsQueryID is the one that this browser session sent
                sessionStorage.setItem('EnhancedTableUpdateStateData', parseInt(sessionStorage.getItem('EnhancedTableUpdateStateData'), 10) + 1);
                if (data[0]){
                    if ((data[0].totalNumberInQuery) || (data[0].totalNumberInQuery > 0)) {
                        if(this.state.eventPageValue*12-11 > data[0].totalNumberInQuery) {
                            let newPage = data[0].totalNumberInQuery/12;
                            if (data[0].totalNumberInQuery<12) newPage=1;
                            this.setState({eventPageValue: newPage}, this.fetchEvents)
                        }
                        console.log("total:", data[0].totalNumberInQuery, "page: ", this.state.eventPageValue*12-11)
                        this.setState({ totalNumberInQuery: data[0].totalNumberInQuery, numbersLoaded: true });
                    } else {
                        const newData = [];
                        data.map((event) => {
                            newData.push(createData(event._id,
                                event.source,
                                event.message,
                                this.formatEventSeverity(event.severity),
                                this.formatTimestamp(event.timestamp)));
                                this.setState({eventsLoaded: true})
                            return null;
                        })
                        if (data[0].totalNumberInQuery === 0){
                            this.props.setLoading(false)
                            this.setState({data: newData, totalNumberInQuery: 0})
                        } else {
                            this.props.setLoading(false)
                            this.setState({ data: newData });
                        }
                    }
                    if (this.state.numbersLoaded && this.state.eventsLoaded) this.setState({eventsLoading: false})
                }
            }
        } 
    }

    /**
     * Format timestamp to timezone
     * @param {*} dateTime date
     */
    formatTimestamp(dateTime) {
        const result = moment(dateTime).tz(this.state.timezone).format('YYYY-MM-DD HH:mm:ss a');
        return result;
    }

    /**
     * Sets enumerated severity level
     * @param {number} severity severity level
     */
    // eslint-disable-next-line class-methods-use-this
    formatEventSeverity(severity) {
        let result = '';
        switch (severity) {
            case 1:
                result = 'Info';
                break;
            case 2:
                result = 'Status';
                break;
            case 3:
                result = 'Alarm';
                break;
            case 4:
                result = 'Fault';
                break;
            default:
                result = 'Debug';
                break;
        }
        return result;
    }

    /**
     * Sets event after filter
     * @param {*} date date to check
     */
    handleEventDateAfterChange = (date) => {
        const newAfterDate = date.toISOString().split('T')[0];
        let beforeDate = this.state.eventDateRangeBefore
            ? this.state.eventDateRangeBefore.toISOString().split('T')[0]
            : new Date().toISOString().split('T')[0];
        let laterDate;
        beforeDate = beforeDate < newAfterDate ? date : this.state.eventDateRangeBefore;
        if (!beforeDate) {
            // if no later date has been set yet, we will enter one that is seven days out
            laterDate = new Date(Number(date));
            laterDate.setDate(date.getDate() + 7);
            beforeDate = laterDate;
        }
        if (beforeDate.toString() === date.toString()) {
            // because beforeDate needs to be one day after afterDate,
            // we add one if user has selected equal dates
            laterDate = new Date(Number(date));
            laterDate.setDate(date.getDate() + 1);
            beforeDate = laterDate;
        }
        if (date) {
            this.setState({
                eventDateRangeAfter: date,
                eventDateRangeBefore: beforeDate,
                eventPageValue: 1,
            }, this.fetchEvents);
        }
    };

    /**
     * Sets event before filter
     * @param {*} date date to check
     */
    handleEventDateBeforeChange = (date) => {
        const newBeforeDate = date.toISOString().split('T')[0];
        let afterDate = this.state.eventDateRangeAfter
            ? this.state.eventDateRangeAfter.toISOString().split('T')[0]
            : new Date().toISOString().split('T')[0];
        const earlierDate = new Date(Number(date));
        earlierDate.setDate(date.getDate() - 1);
        // if the before date is less than or equal to the after date,
        // we make the after date one day before the before date
        afterDate = newBeforeDate <= afterDate ? earlierDate : this.state.eventDateRangeAfter;
        if (date) {
            this.setState({
                eventDateRangeAfter: afterDate,
                eventDateRangeBefore: date,
                eventPageValue: 1,
            }, this.fetchEvents);
        }
    };

    /**
     * Goes to previous page
     */
    goToPrevPage() {
        const thePage = this.state.eventPageValue > '1' ? (this.state.eventPageValue - 1).toString() : '1';
        this.setState({
            eventPageValue: thePage,
            numbersLoaded: true
        }, this.fetchEvents);
    }

    /**
     * Goes to next page
     */
    goToNextPage() {
        const thePage = parseInt(this.state.totalNumberInQuery / 12, 10)
            + 1 > this.state.eventPageValue ? (Number(this.state.eventPageValue)
                + 1).toString() : this.state.eventPageValue;
        this.setState({
            eventPageValue: thePage,
            numbersLoaded: true
        }, this.fetchEvents);
    }

    /**
     * Sorts event based on event target
     * @param {*} event
     */
    handleEventSortClick = (event) => {
        let theSortValue = { timestamp: 1 };
        switch (event.target.value) {
            case 'source':
                if ((this.state.eventSortValue).source === 1) {
                    theSortValue = { source: -1, timestamp: -1 };
                } else {
                    theSortValue = { source: 1, timestamp: -1 };
                }
                break;
            case 'message':
                if ((this.state.eventSortValue).message === 1) {
                    theSortValue = { message: -1, timestamp: -1 };
                } else {
                    theSortValue = { message: 1, timestamp: -1 };
                }
                break;
            case 'severity':
                if ((this.state.eventSortValue).severity === 1) {
                    theSortValue = { severity: -1, timestamp: -1 };
                } else {
                    theSortValue = { severity: 1, timestamp: -1 };
                }
                break;
            case 'timestamp':
                if ((this.state.eventSortValue).timestamp === 1) {
                    theSortValue = { timestamp: -1 };
                } else {
                    theSortValue = { timestamp: 1 };
                }
                break;
            default:
                theSortValue = { timestamp: -1 };
                break;
        }
        this.setState({
            eventSortValue: theSortValue,
            eventPageValue: 1,
        }, this.fetchEvents);
    };

    /**
     * Sets severity filter
     * @param {*} event 
     */
    async handleSelect(event){
        let bool = await this.setState({
            eventSeverityValue: event.target.value
        }, this.fetchEvents)
        if (bool) this.props.setLoading(false)
    }

    handleSearch(event) {
        this.setState( {eventPageValue: 1}, this.fetchEvents)

    }

    handleTextBox(event) {
        this.setState({
            textBoxText: event.target.value
        })
    }

    /**
     * Checks if ID is selected
     * @param {*} id
     */
    isSelected = (id) => this.state.selected.indexOf(id) !== -1;

    render() {
        sessionStorage.setItem('EnhancedTableRender', parseInt(sessionStorage.getItem('EnhancedTableRender'), 10) + 1);
        // eslint-disable-next-line react/prop-types
        const { classes } = this.props;
        // eslint-disable-next-line no-shadow
        const { data, isLoading, eventSortValue } = this.state;
        const timestampClassName = (Object.keys(eventSortValue).length > 1
            ? classNames(classes.filterButton, classes.arrow, classes.grayArrow) : classNames(classes.filterButton, classes.arrow));
        return (
            <>
                {!isLoading
                    && <>
                        <div className={classes.eventsFiltering}>
                            <div style={{display: 'flex', flexDirection: 'row'}}></div>
                            <div className={classes.labelText}>Only view events on or after:</div>
                                <DatePicker monthPlaceholder={this.state.initialDateAfter.substring(5,7)} dayPlaceholder={this.state.initialDateAfter.substring(8,10)} yearPlaceholder={this.state.initialDateAfter.substring(0,4)} clearIcon={null} className={classes.datepicker} id='dateRangeAfter' value={this.state.eventDateRangeAfter} onChange={this.handleEventDateAfterChange} />
                            <div className={classes.labelText}>and before:</div><DatePicker monthPlaceholder={this.state.initialDateBefore.substring(5,7)} dayPlaceholder={this.state.initialDateBefore.substring(8,10)} yearPlaceholder={this.state.initialDateBefore.substring(0,4)} clearIcon={null} className={classes.datepicker} id='dateRangeBefore' value={this.state.eventDateRangeBefore} onChange={this.handleEventDateBeforeChange} />
                            <br />
                            <br />
                            <FormControl style={{width: '26%', marginLeft: '14%'}}
                            variant="outlined"
                            >
                                <InputLabel>Event Severity</InputLabel>
                                <Select
                                multiple
                                label="Event Severity"
                                onChange={this.handleSelect}
                                value={this.state.eventSeverityValue}
                                >
                                    <MenuItem value={1}>Info</MenuItem>
                                    <MenuItem value={2}>Status</MenuItem>
                                    <MenuItem value={3}>Alarm</MenuItem>
                                    <MenuItem value={4}>Fault</MenuItem>
                                </Select> 
                            </FormControl>
                            <TextField
                            variant="outlined"
                            label="Search for message"
                            style={{paddingRight: 20, marginLeft: '2%', zIndex: 0}}
                            onChange={this.handleTextBox}>
                            </TextField>
                            <Button
                            variant="contained"
                            color="primary"
                            style={{marginTop: 10}}
                            onClick={this.handleSearch}>
                                apply
                            </Button>
                        </div>
                        <div>
                            <table className={classes.table}>
                                <thead>
                                    <tr className={classes.row}>
                                        <th className={classes.column}>
                                            <button className={classes.filterButton} value='source' onClick={this.handleEventSortClick}>Source</button>{eventSortValue.source === -1 && <button className={classNames(classes.filterButton, classes.arrow)} value='source' onClick={this.handleEventSortClick}>/\</button>}{eventSortValue.source === 1 && <button className={classNames(classes.filterButton, classes.arrow)} value='source' onClick={this.handleEventSortClick}>\/</button>}
                                        </th>
                                        <th className={classes.column}>
                                            <button className={classes.filterButton} value='message' onClick={this.handleEventSortClick}>Message</button>{eventSortValue.message === -1 && <button className={classNames(classes.filterButton, classes.arrow)} value='message' onClick={this.handleEventSortClick}>/\</button>}{eventSortValue.message === 1 && <button className={classNames(classes.filterButton, classes.arrow)} value='message' onClick={this.handleEventSortClick}>\/</button>}
                                        </th>
                                        <th className={classes.column}>
                                            <button className={classes.filterButton} value='severity' onClick={this.handleEventSortClick}>Severity</button>{eventSortValue.severity === -1 && <button className={classNames(classes.filterButton, classes.arrow)} value='severity' onClick={this.handleEventSortClick}>/\</button>}{eventSortValue.severity === 1 && <button className={classNames(classes.filterButton, classes.arrow)} value='severity' onClick={this.handleEventSortClick}>\/</button>}
                                        </th>
                                        <th className={classes.column}>
                                            <button className={classes.filterButton} value='timestamp' onClick={this.handleEventSortClick}>Timestamp</button>{eventSortValue.timestamp === -1 && <button className={timestampClassName} value='timestamp' onClick={this.handleEventSortClick}>/\</button>}{eventSortValue.timestamp === 1 && <button className={timestampClassName} value='timestamp' onClick={this.handleEventSortClick}>\/</button>}
                                        </th>
                                    </tr>
                                </thead>
                                {this.state.eventsLoading ? <ActivityIndicator leftMargin={'50%'} topMargin={'25%'} color='#000000'/> : <></>}
                                <tbody>
                                    {data.map((event) => {
                                        const isSelected = this.isSelected(event.id);
                                        let rowColor = classes.default;
                                        if (event.severity === 'Status') {
                                            rowColor = classes.status;
                                        }
                                        if (event.severity === 'Alarm') {
                                            rowColor = classes.alarm;
                                        }
                                        if (event.severity === 'Fault') {
                                            rowColor = classes.fault;
                                        }
                                        const rowColorPlusOtherClasses = `${rowColor} ${classes.row}`;
                                        return (
                                            <tr key={event.id} selected={isSelected}
                                                className={rowColorPlusOtherClasses}>
                                                <td className={classes.column}>{event.source}</td>
                                                <td className={classes.column}>{event.message}</td>
                                                <td className={classes.column}>{event.severity}</td>
                                                <td className={classes.column}>{event.timestamp}</td>
                                            </tr>
                                        );
                                    })}
                                </tbody>
                            </table>
                            <div className={classes.pageDisplay}>
                                Showing record {(this.state.eventPageValue * 12) - 11}-{this.state.eventPageValue * 12 > this.state.totalNumberInQuery ? this.state.totalNumberInQuery : this.state.eventPageValue * 12} of {this.state.totalNumberInQuery} <button className={classes.button} onClick={this.goToPrevPage.bind(this)}>PREVIOUS</button> <button className={classes.button} onClick={this.goToNextPage.bind(this)}>NEXT</button>
                            </div>
                        </div>
                    </>}
            </>
        );
    }
}
EnhancedTable.propTypes = {
    classes: PropTypes.object,
};
export default withStyles(LoadingHOC(EnhancedTable), STYLES_EVENTS);
