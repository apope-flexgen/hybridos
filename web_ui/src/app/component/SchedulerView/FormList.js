import React from 'react';

import { withStyles } from 'tss-react/mui';
import { isDayDST } from '../../../AppFunctions'
import moment from 'moment-timezone';


import Typography from '@mui/material/Typography';
import InputLabel from '@mui/material/InputLabel';
import MenuItem from '@mui/material/MenuItem';
import FormHelperText from '@mui/material/FormHelperText';
import FormControl from '@mui/material/FormControl';
import FormControlLabel from '@mui/material/FormControlLabel';
import Select from '@mui/material/Select';
import { STYLES_FORMLIST } from '../../styles';
import InputAdornment from '@mui/material/InputAdornment';
import TextField from '@mui/material/TextField';
import Button from '@mui/material/Button';
import IconButton from '@mui/material/IconButton';
import Checkbox from '@mui/material/Checkbox';

import AvTimerIcon from '@mui/icons-material/AvTimer';
import CheckIcon from '@mui/icons-material/Check';
import CloseIcon from '@mui/icons-material/Close';
import DeleteIcon from '@mui/icons-material/Delete';

const DEFAULT_WIDTH = '240px';
class FormList extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            dropDown: '',
            input: {},
            startTime: '07:30',
            duration: '',
            receivedEvent: null,
            receivedEventDayIsToday: false,
            showConfirmation: false
        }
        this.startTime = React.createRef();
        this.duration = React.createRef();
        this.mode = React.createRef();
        this.actualInput = React.createRef();

        this.submitEventButtonPressed = this.submitEventButtonPressed.bind(this);
        this.cancelButtonPressed = this.cancelButtonPressed.bind(this);
        this.onDeletePressed = this.onDeletePressed.bind(this);
        this.confirmation = this.confirmation.bind(this);
        this.cancelDelete = this.cancelDelete.bind(this);
    }

    /**
     * Necessary because users can click mutliple events in a row without exiting the FormList
    */
    static getDerivedStateFromProps(props, state) {
        if (props.selectedEvent == null && !props.addEventButtonSelected) {
            const theTime = new Date();    
            const convertTime = moment(theTime).tz(props.timezone).format("HH:mm");

            return {
                dropDown: '',
                input: {},
                startTime: convertTime,
                duration: '',
                receivedEvent: null,
                showConfirmation: false
            }
        }
        if (props.selectedEvent!=null) {
            let realTime = props.selectedEvent.start_time;
            let whichDay = props.selectedEvent == null ? props.isTodaySelected : props.selectedEventDayIsToday;
            if (isDayDST(whichDay, true, props.timezone)) {
                if (realTime >= 120) {
                    realTime += 60;
                }
            }
            if (isDayDST(whichDay, false, props.timezone)) {
                if (realTime >= 120) {
                    realTime -= 60;
                }
            }
            if (realTime < 0) {
                realTime += 1440;
            }
            let hours = (Math.floor(realTime/60)).toString();
            let minutes = (realTime%60).toString();
            
            if (minutes.length == 1) {
                minutes = '0' + minutes;
            }
            if (hours.length == 1) {
                hours = '0' + hours;
            }
            let startTime = hours+':'+minutes;
            let newInput = {};
            for (let i = 0; i < props.modeData[props.selectedEvent.mode].variables.length; i++) {
                newInput[props.modeData[props.selectedEvent.mode].variables[i].id] = props.selectedEvent.variables[props.modeData[props.selectedEvent.mode].variables[i].id] != null ? props.selectedEvent.variables[props.modeData[props.selectedEvent.mode].variables[i].id].toString() :
                props.modeData[props.selectedEvent.mode].variables[i].value.toString();                
            }
            if (props.selectedEvent!=state.receivedEvent) {
                return{
                    dropDown: props.selectedEvent.mode,
                    input: newInput,
                    startTime: startTime,
                    duration: props.selectedEvent.duration,
                    receivedEvent: props.selectedEvent,
                    receivedEventDayIsToday: props.selectedEventDayIsToday,
                    showConfirmation: false
                }
            } else {
                return null;
            }
        } else {
            if(props.selectedEvent!=state.receivedEvent) {
                const theTime = new Date();    
                const convertTime = moment(theTime).tz(props.timezone).format("HH:mm");
                return{
                    dropDown: '',
                    input: {},
                    startTime: convertTime,
                    duration: '', 
                    receivedEvent: null,
                    receivedEventDayIsToday: false,
                    showConfirmation: false
                }  
            }
        }
    }

    /**
     * Resets the form
    */
    reset() {
        const theTime = new Date();    
        const convertTime = moment(theTime).tz(this.props.timezone).format("HH:mm");
        this.setState({
            dropDown: '',
            input: {},
            startTime: convertTime,
            duration: '',
            receivedEvent: null,
            showConfirmation: false
        })
    }

    /**
     * Upon changing modes, newInput is updated to be a map with the variables as keys and their default values as the values
    */
    handleDropDown = (event) => {
        let newInput = {};
        for (let i = 0; i < this.props.modeData[event.target.value].variables.length; i++) {
                newInput[this.props.modeData[event.target.value].variables[i].id] = this.props.modeData[event.target.value].variables[i].value;            
        }
        this.setState({
            dropDown: event.target.value,
            input: newInput
        });
    }

    /**
     * For variables of type int, this ensures what is being entered is an int and updating the input map
    */
    handleInt = (event, id) => {
        let number = event.target.value;
        let minusCount = 0;
        for(let i=0; i<number.length; i++) {
            if (i==0 && number.charAt(i)=='-' && minusCount<1) {
                minusCount++;
                continue;
            }
          
            if (number.charAt(i)=='+' || number.charAt(i)=='.' || number.charAt(i)=='e' || '0'>number.charAt(i) || number.charAt(i)>'9') {
                return;
            }
        }
        let newInput = JSON.parse(JSON.stringify(this.state.input));
        newInput[id] = event.target.value;
        this.setState({
            input: newInput
        });
    }

    /**
     * For variables of type float, this ensures what is being entered is an int and updating the input map
    */
    handleFloat = (event, id) => {
        let number = event.target.value;
        let minusCount = 0;
        let decimalCount = 0;
        for(let i=0; i<number.length; i++) {
            if (i==0 && number.charAt(i)=='-' && minusCount<1) {
                minusCount++;
                continue;
            }
            if (number.charAt(i)=='.' && decimalCount<1) {
                decimalCount++;
                continue;
            }
            if (number.charAt(i)=='+' || number.charAt(i)=='e' || '0'>number.charAt(i) || number.charAt(i)>'9') {
                return;
            }
        }
        let newInput = JSON.parse(JSON.stringify(this.state.input));
        newInput[id] = event.target.value;
        this.setState({
            input: newInput
        });
    }

    /**
    * Updates start time upon changing the input field
    */
    handleStartTime = (event) => {
        let inputTime = event.target.value;
        this.setState({
            startTime: inputTime
        });
    }

    /**
     * Returns true if start time is invalid due to the start time being in the past
     */
    isInvalidStartTimeBecauseOfCurrentTime() {
        if (!this.props.isTodaySelected) {
            return false;
        }
        if (this.state.receivedEvent != null && !this.state.receivedEventDayIsToday) {
            return false;
        }
        if (this.state.startTime == '') {
            return true;
        }
        let time = this.state.startTime.split(':');

        const theTime = new Date();    
        const convertTime = moment(theTime).tz(this.props.timezone).format("YYYY-MM-DD HH:mm:ss");
        const targetTime = new Date(convertTime);

        targetTime.setHours(parseInt(time[0]));
        targetTime.setMinutes(parseInt(time[1]));

        const theTime2 = new Date();    
        const convertTime2 = moment(theTime2).tz(this.props.timezone).format("YYYY-MM-DD HH:mm:ss");
        const currentTime = new Date(convertTime2);
        if (this.props.isTodaySelected) {
            if (currentTime > targetTime){
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    /**
     * Returns true if start time exists (on spring forward days an hour of time does not exist)
     */
    isValidDespiteDST() {
        let time = this.state.startTime.split(':');
        let minutes = parseInt(time[0]) * 60 + parseInt(time[1]);

        let isToday = this.props.selectedEvent != null ? this.props.selectedEventDayIsToday : this.props.isTodaySelected;
        if (isDayDST(isToday, true, this.props.timezone) && (minutes >= 120 && minutes < 180)) {
        // if (checkIfTimeExists(minutes, isToday, this.props.time, this.props.timezone)) {
            return false; //DST happening
        } else if (isDayDST(isToday, true, this.props.timezone) && !(minutes >= 120 && minutes < 180)) {
            return true;
        
        } else {
            return true;
        }
    }

    /**
     * Returns true if start time is invalid due to the start time being in the past, being during spring forward, or is an already occurring event, or if it overlaps with another event
     */
    isInvalidStartTime() {
        
        if (this.startTimeIsDisabledForAlreadyOccuringEvent()) {
            return false;
        }
        if (this.isDisabled() || this.state.startTime == '') {
            return false;
        }
        if (!this.isValidDespiteDST()) {
            return true; //DST happening
        }
        if (this.isInvalidStartTimeBecauseOfCurrentTime()) {
            
            return true;
            return this.checkOverlappingStartTime();
        } else {
            return this.checkOverlappingStartTime();
        }
    }

    /**
     * Returns true if start time overlaps with another event's duration
     */
    checkOverlappingStartTime() {
        return this.props.isOverlappingStartTime(this.state.startTime, this.state.duration);
    }

    /**
     * Returns true if start time does not overlap with another event's duration
     */
    checkOverlappingDuration() {
        return !this.props.isOverlappingDuration(this.state.startTime, this.state.duration, )
    }

    /**
     * Returns true if event's duration does not cause it to pass current time
     */
    durationPutsEventInPast() {
        if (this.state.receivedEvent != null) {
            let splitArray = this.state.startTime.split(':');
            let hours = parseInt(splitArray[0]);
            let minutes = parseInt(splitArray[1]);
            let startTimeInMinutes = hours * 60 + minutes;
            let duration = parseInt(this.state.duration)
            if (this.state.receivedEventDayIsToday && (startTimeInMinutes + duration) / 1440 < ((this.props.time.getHours() * 60 + this.props.time.getMinutes()) / 1440)) {
                return true;
            } else if (!this.state.receivedEventDayIsToday && (1440 + startTimeInMinutes + duration) / 1440 < ((this.props.time.getHours() * 60 + this.props.time.getMinutes()) / 1440)) {
                return true;
            }
        }
        return false;
    }

    /**
     * Allows only integers to be placed into duration field
     */
    handleDuration = (event) => {
        let number = event.target.value
        for(let i=0; i<number.length; i++) {
            if ('0'>number.charAt(i) || number.charAt(i)>'9') {
                return;
            }
        }
        this.setState({
            duration: event.target.value
        });
    }

    /**
     * Returns true if duration is less than 24 hours and greater than 0
     */
    isValidDurationLessThan24Hours() {
        
        if(parseInt(this.state.duration) >= 1440 || parseInt(this.state.duration)<=0) {
            return false;
        } else {
            return true;
        }
    }

    
    /**
     * Returns true if duration is within 24 hours and doesn't cause overlap
     */
    isValidDuration() {
        if (this.isDisabled()) {
            return true;
        }
        if(!this.isValidDurationLessThan24Hours()) {
            return false
        } else {
            return this.checkOverlappingDuration() && !this.durationPutsEventInPast();
        }
    }

    /**
     * Changes a boolean in one of the variables for the mode
     * @param num which field number to change
     */
    handleBoolean = (event, num) => {
        let newInput = JSON.parse(JSON.stringify(this.state.input));
        newInput[num] = event.target.checked;
        this.setState({
            input: newInput
        });

    }

    /**
     * Changes a string in one of the variables for the mode
     * @param num which field number to change
     */
    handleString = (event, num) => {
        let newInput = JSON.parse(JSON.stringify(this.state.input));
        newInput[num] = event.target.value;
        this.setState({
            input: newInput
        });

    }

    /**
     * When dropdown changes, this is called to render the proper fields
     * @param {string} mode - which mode
     */
    renderMode(mode) {
        if (this.props.modeData[mode].variables == null) {
            return;
        }
        let forms = [];
        for (let i = 0; i < this.props.modeData[mode].variables.length; i++) {
            forms.push(this.renderInput(this.props.modeData[mode].variables[i]))
        }
        return forms;

    }

    /**
     * Creates one field based on input's type (string, boolean, number)
     * @param {object} input - which variable
     */
    renderInput(input) {
        switch(input.type) {
            case 'String':
                return (<TextField
                    style={{ marginTop: '10px', width: DEFAULT_WIDTH}}
                    label={input.name}
                    helperText={input.id}
                    variant="outlined"
                    
                    onChange={(e) => this.handleString(e, input.id)}
                    value={this.state.input[input.id]}
                    />)
                break;
            case 'Int':
                return (<TextField
                    style={{ marginTop: '10px', width: DEFAULT_WIDTH}}
                    label={input.name}
                    InputProps={{
                        endAdornment: <InputAdornment position="end">{input.unit}</InputAdornment>,
                    }}
                    helperText={input.id}
                    variant="outlined"
                    
                    onChange={(e) => this.handleInt(e, input.id)}
                    value={this.state.input[input.id]}
                    />)
                break;
            case "Float":
                return (<TextField
                    style={{ marginTop: '10px', width: DEFAULT_WIDTH}}
                    inputRef={this.actualInput}
                    label={input.name}
                    InputProps={{
                        endAdornment: <InputAdornment position="end">{input.unit}</InputAdornment>,
                    }}
                    helperText={input.id}
                    variant="outlined"
                    
                    onChange={(e) => this.handleFloat(e, input.id)}
                    value={this.state.input[input.id]}
                    />)
                break;
            case "Bool":
                return (
                    <FormControlLabel
                        control={
                        <Checkbox
                            inputRef={this.actualInput}
                            onChange={(e) => this.handleBoolean(e, input.id)}
                            checked={this.state.input[input.id] == true || this.state.input[input.id] == 'true'}
                            color="primary"
                        />
                        }
                        label={input.name}
                    />
                )
                break;
        }
    }

    /**
     * Returns true if the form list should not display (no event selected and add event button not pressed)
     */
    isDisabled() {
        return this.props.selectedEvent == null && !this.props.addEventButtonSelected;
        
    }

    /**
     * Returns title for form
     */
    getTitle() {
        if (this.props.selectedEvent != null) {
            return "Edit " + (this.state.receivedEventDayIsToday ? "Today's" : "Tomorrow's") + " Event";
        } else if (this.props.addEventButtonSelected) {
            if (this.props.isTodaySelected) {
                return "New Event for Today";
            } else {
                return "New Event for Tomorrow";
            }
        } else if (this.isDisabled()) {
            return "Select or Add New Event";
        }
    }

    /**
     * Returns gray when form should not show
     */
    layerWhenDisabled() {
        if (this.isDisabled()) {
            return 'rgba(220, 220, 220, .5)';
        } else {
            return '';
        }
    }

    /**
     * Creates the event JSON object and passes it up to Scheduler to deal with
     */
    submitEventButtonPressed() {
        let submittedStartTime = this.state.startTime;
        let submittedDuration = this.state.duration;
        let submittedMode = this.state.dropDown;
        let submittedInput = JSON.parse(JSON.stringify(this.state.input));;

        let keys = Object.keys(submittedInput);

        let typeMapping = {};

        for (let j = 0; j < this.props.modeData[this.state.dropDown].variables.length; j++) {
            typeMapping[this.props.modeData[this.state.dropDown].variables[j].id] = this.props.modeData[this.state.dropDown].variables[j].type;
        }
        
        for (let i = 0; i < keys.length; i++) {

            if (typeMapping[keys[i]] == 'Int') {
                submittedInput[keys[i]] = parseInt(submittedInput[keys[i]])
            }
            if (typeMapping[keys[i]] == 'Float') {
                submittedInput[keys[i]] = parseFloat(submittedInput[keys[i]])
            }
            if (typeMapping[keys[i]] == 'Bool') {
                submittedInput[keys[i]] = submittedInput[keys[i]] == true ||  submittedInput[keys[i]] == 'true' ? true : false;
            }
        }

        let submittedEvent = {
            start_time: submittedStartTime,
            duration: submittedDuration,
            mode: submittedMode,
            variables: submittedInput
        }
        this.props.submitEvent(submittedEvent);

        this.reset();
    }

    /**
     * Closes form
     */
    cancelButtonPressed() {
        this.reset();
        this.props.cancelAddEvent();
    }

    /**
     * Returns true when everything is filled out properly
     */
    submitAvailable() {
        let keys = Object.keys(this.state.input);
        for (let i = 0; i < keys.length; i++) {
            let varInModeData;

            let type = this.props.modeData[this.state.dropDown]['variables'][i]['type'];
            if ((type == 'Float' || type == 'Int' || type == 'String') && this.state.input[keys[i]] == '') {
                return false;
            }
        }
        return !this.isDisabled() 
        && this.state.startTime != '' 
        && this.state.duration != '' 
        && this.state.dropDown != '' 
       
        && this.isValidDuration()
        && !this.isInvalidStartTime();
    }

    /**
     * Initates confirmation for deleting event
     */
    onDeletePressed() {
        if(this.state.receivedEvent==null) {
            this.cancelButtonPressed();
        } else {
            this.reset();
            this.props.deleteEvent();
        }
    }
    
    /**
     * Turns on confirmation state (check and x clickable)
     */
    confirmation() {
        this.setState({
            showConfirmation: true
        });
    }

    /**
     * Turns off confirmation state (check and x unclickable)
     */
    cancelDelete() {
        this.setState({
            showConfirmation: false
        });
    }

    /**
     * Returns bright color when submit available, otherwise gray
     */
    submitColors() {
        if(!this.submitAvailable()) {
            return '#E0E0E0';
        } else {
            return '#3F51B5';
        }
    }

    /**
     * Returns proper message for bad start time
     */
    errorMessageForStartTime() {
        let currentTimeIssue = this.isInvalidStartTimeBecauseOfCurrentTime();
        let overlappingIssue = this.checkOverlappingStartTime();
        let dstIssue = !this.isValidDespiteDST();
        if (currentTimeIssue) {
            return "Invalid time: must be after current time";
        } else if (!currentTimeIssue && dstIssue){
            return "Invalid time: time doesn't exist (DST)"
        } else if (!currentTimeIssue && overlappingIssue) {
            return "Invalid time: must not overlap events";
        } else {
            return "Invalid time: must be after current time";
        }
    }

    /**
     * Returns proper message for a bad duration
     */
    errorMessageForDuration() {
        let tooLongError = !this.isValidDurationLessThan24Hours();
        let overlappingIssue = !this.checkOverlappingDuration();
        if (this.durationPutsEventInPast()) {
            return 'Invalid duration: must end event after current time'
        }
        if (tooLongError) {
            return "Invalid duration: must be less than 24 hours";
        } else if (!tooLongError && overlappingIssue) {
            return "Invalid duration: must not overlap events";
        } else {
            return "Invalid duration: must be less than 24 hours";
        }

    }

    /**
     * Returns submit if creating new event, save for editing event
     */
    submitOrSave() {
        if (this.state.receivedEvent == null) {
            return "SUBMIT";
        } else {
            return "SAVE"
        }
    }

    /**
     * Returns 1500 minutes on a fall back day, 1440 for normal
     */
    getProperTimeInDay() {
        let whichDay = this.props.receivedEvent == null ? this.props.isTodaySelected : this.state.receivedEventDayIsToday;
      if (isDayDST(whichDay, false, this.props.timezone)) {
          return 1440 + 60;
      } else {
          return 1440;
      }
    }

    /**
     * Used under duration field to show when the event will finish
     */
    getTimes(timeSinceMidnight) {
        let startingHours;
        let startingMinutes;
        let yesterdayOrTomorrowText = "";
        if (timeSinceMidnight < 0) {
            startingHours = Math.floor((1440 + timeSinceMidnight) / 60);
            startingMinutes = (1440 + timeSinceMidnight) % 60;
            yesterdayOrTomorrowText = "(yesterday)"
        } else if (timeSinceMidnight >= 0 && timeSinceMidnight <= 1440) {
            startingHours = Math.floor(timeSinceMidnight / 60);
            startingMinutes = timeSinceMidnight % 60;
        } else {
            startingHours = Math.floor((timeSinceMidnight - 1440) / 60);
            startingMinutes = (timeSinceMidnight - 1440) % 60;
            yesterdayOrTomorrowText = "(tomorrow)"
        }
        
        if (startingMinutes < 10) {
            startingMinutes = "0" + startingMinutes;
        }
        var AMorPM = startingHours < 12 ? "AM" : "PM";
        if (timeSinceMidnight == 1440) {
            AMorPM = "AM";
        }
        if (startingHours === 0) {
            startingHours = 12;
        } else if (startingHours > 12) {
            startingHours -= 12;
        }
        return `${startingHours}:${startingMinutes} ${AMorPM} ${yesterdayOrTomorrowText}`;
    }

    /**
     * Returns either nothing, an error message, or an end time for an event
     */
    getTimeHelperTimeForDurationField() {
        if (this.state.duration == '' || this.state.startTime == '') {
            return '';
        } else {
            let splitArray = this.state.startTime.split(':');
            let hours = parseInt(splitArray[0]);
            let minutes = parseInt(splitArray[1]);
            let adjustedStartTime = hours * 60 + minutes;
            let adjustedDuration = parseInt(this.state.duration);
            let whichDay = this.props.selectedEvent == null ? this.props.isTodaySelected : this.props.selectedEventDayIsToday;
            if (whichDay == true && isDayDST(false, true, this.props.timezone)) { //today event, tomorrow is spring forward
                if ( adjustedStartTime - 1440 < 120 && adjustedStartTime - 1440 + adjustedDuration >= 120) {
                    adjustedDuration += 60;
                }
            }
            if (whichDay == true && isDayDST(false, false, this.props.timezone)) { //today event, tomorrow is fall back
                if ( adjustedStartTime - 1440 < 120 && adjustedStartTime - 1440 + adjustedDuration >= 120) {
                    adjustedDuration -= 60;
                }
            }
            if (isDayDST(whichDay, true, this.props.timezone)) {
                if ( hours * 60 + minutes < 120 &&  hours * 60 + minutes + adjustedDuration >= 120) {
                    adjustedDuration += 60;
                }
            }
            if (isDayDST(whichDay, false, this.props.timezone)) { //fall back today?
                if ( hours * 60 + minutes < 120 &&  hours * 60 + minutes + adjustedDuration >= 120) {
                    adjustedDuration -= 60;
                }
            }
            
            return this.getTimes(adjustedDuration + adjustedStartTime);
        }
    }

    errorMessageForStartTimeAlreadyPassed() {
        if (this.startTimeIsDisabledForAlreadyOccuringEvent()) {
            return 'Start time has already passed';
        } else {
            return '';
        }
    }

    startTimeIsDisabledForAlreadyOccuringEvent() {
        if (this.state.receivedEvent == null) {
            return false;
        } else {
            let startTimeInMinutes = this.state.receivedEvent.start_time
            if (this.state.receivedEventDayIsToday && (startTimeInMinutes) / 1440 <= ((this.props.time.getHours() * 60 + this.props.time.getMinutes()) / 1440)) {
                return true;
            } else if (!this.state.receivedEventDayIsToday && (1440 + startTimeInMinutes) / 1440 <= ((this.props.time.getHours() * 60 + this.props.time.getMinutes()) / 1440)) {
                return true;
            }
        }        
    }

    getHelperTextForDropDown() {
        if (this.startTimeIsDisabledForAlreadyOccuringEvent()) {
            return "Start time has passed";
        } else {
            return "";
        }
    }

    render() {
        let modeDropDown = [];
        let modeKeys = Object.keys(this.props.modeData);
        for (let i = 0; i < modeKeys.length; i++) {
            if (modeKeys[i] == 'default') {
                continue;
            }
            modeDropDown.push(
                <MenuItem value={modeKeys[i]}>{modeKeys[i]}</MenuItem>
            )
        }
        let startTimeError = false;
        if (this.startTimeIsDisabledForAlreadyOccuringEvent()) {
            startTimeError = false;
        } else if (this.isInvalidStartTime()) {
            startTimeError = true;
        }
        
        return (
            <div style={{ height: '100%', position: 'absolute', left: '0', background: `${this.layerWhenDisabled()}`, width: '100%', display: 'flex', alignItems: 'center', flexDirection: 'column'}}>
                <div style={{ height: '10%'}}>
                    <Typography style={{ fontWeight:'bold', fontSize: '16px', marginTop: '12px', marginBottom: '8px' }}>
                    { this.getTitle() }
                    </Typography>   
                </div>
                {(this.state.receivedEvent != null || this.props.addEventButtonSelected == true) &&
                <div style={{ height: '80%', width: '100%', display: 'flex', alignItems: 'center', flexDirection: 'column', overflowY: 'scroll'}}>
                    <div style={{ display: 'flex', marginTop:'10px' }}>
                        <form>
                            <TextField
                                style={{ width: DEFAULT_WIDTH}}
                                id="time"
                                inputRef={this.startTime}
                                label="Start Time"
                                type="time"
                                variant="outlined"
                                error={startTimeError}
                                helperText= {
                                    this.isInvalidStartTime() ? `${this.errorMessageForStartTime()}` : `${this.errorMessageForStartTimeAlreadyPassed()}`
                                }
                                InputLabelProps={{
                                shrink: true,
                                }}
                                inputProps={{
                                step: 300, // 5 min
                                }}
                                onChange={this.handleStartTime}
                                
                                disabled={this.isDisabled() || this.startTimeIsDisabledForAlreadyOccuringEvent()}
                                value={this.state.startTime}                                
                            />
                        </form>
                        <TextField
                            inputRef={this.duration}
                            style={{marginLeft: '10px', width: DEFAULT_WIDTH}}
                            label="Duration"
                            InputProps={{
                                endAdornment: <InputAdornment position="end"><AvTimerIcon fontSize="small" style={{color: '#000000'}}/></InputAdornment>,
                            }}
                            variant="outlined"
                            error= {!this.isValidDuration()}
                            helperText = {
                                !this.isValidDuration() ? `${this.errorMessageForDuration()}` :  `${this.getTimeHelperTimeForDurationField()}` 
                            }
                            onChange={this.handleDuration}
                            disabled={this.isDisabled()}
                            value={this.state.duration}                            
                        />
                    </div>

                    <FormControl 
                    disabled={this.isDisabled()}
                    style={{ width: DEFAULT_WIDTH, marginTop: '10px', marginBottom: '10px' }} 
                    variant="outlined">
                        <InputLabel>Type of Event</InputLabel>
                        <Select
                        label="Type of Event"
                        inputRef={this.mode}
                        name="Type of Event"
                        disabled={this.isDisabled() || this.startTimeIsDisabledForAlreadyOccuringEvent()}
                        
                        onChange={this.handleDropDown}
                        value={this.state.dropDown}
                        >

                            {modeDropDown}

                        </Select>
                        {this.startTimeIsDisabledForAlreadyOccuringEvent() &&
                            <FormHelperText>Start time has already passed, mode selection disabled</FormHelperText>}
                    </FormControl>
                

                {this.props.modeData && this.state.dropDown != '' && this.renderMode(this.state.dropDown)}
                </div>
                }

                {(this.state.receivedEvent != null || this.props.addEventButtonSelected == true) &&
                <div style={{ bottom: '0', position: 'absolute', width: '100%'}}>
                    <div style={{ left: '0', bottom: '0', position: 'absolute'}}>
                        <IconButton
                            onClick={() => this.confirmation()}
                            disabled={this.isDisabled() || this.state.showConfirmation}
                            style={{color:'#424142', opacity: `${this.isDisabled() || this.state.showConfirmation ? '25%' : '100%'}`}}
                            size="large"><DeleteIcon/></IconButton>
                        <IconButton
                            onClick={() => this.onDeletePressed()}
                            disabled={this.isDisabled() || !this.state.showConfirmation}
                            style={{color:`${this.state.showConfirmation ? '#4BC940' : '#d1d1d1'}` }}
                            size="large"><CheckIcon/></IconButton>
                        <IconButton
                            onClick={() => this.cancelDelete()}
                            disabled={this.isDisabled() || !this.state.showConfirmation}
                            style={{color:`${this.state.showConfirmation ? '#E33B3B' : '#d1d1d1'}` }}
                            size="large"><CloseIcon/></IconButton>
                    </div>
                    
                    <div style={{ right: '0', bottom: '4px', right: '4px', position: 'absolute', display: 'flex', justifyContent: 'center', alignItems: 'center'}}>
                        <Button onClick={() => this.cancelButtonPressed()} disabled={this.isDisabled()} variant="outlined" color="secondary"style={{marginRight: '10px'}}>CANCEL</Button>
                        <Button onClick={() => this.submitEventButtonPressed()} disabled={!this.submitAvailable()} variant="contained" 
                            color="primary">{this.submitOrSave()}</Button>
                    
                    </div>
                </div>
                }
                
            </div>
        );
    }

}

export default withStyles(FormList, STYLES_FORMLIST);