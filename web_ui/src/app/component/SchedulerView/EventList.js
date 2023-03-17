import React from 'react';
import { withStyles } from 'tss-react/mui';

import Button from '@mui/material/Button';
import IconButton from '@mui/material/IconButton';
import Typography from '@mui/material/Typography'
import Grid from '@mui/material/Grid';

import KeyboardArrowRightIcon from '@mui/icons-material/KeyboardArrowRight';
import KeyboardArrowLeftIcon from '@mui/icons-material/KeyboardArrowLeft';

import { STYLES_EVENTLIST, STYLES_FEATURES } from '../../styles';

import { isDayDST } from '../../../AppFunctions'

class EventList extends React.Component {
    constructor(props) {
        super(props);
        this.selectEvent = this.selectEvent.bind(this);
    }

    /**
     * Returns rgb value of a hex string
     * @param {string} hex - a string in the form of #FFFFFF
     */
    hexToRgb(hex) {
        var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
        return result ? {
          r: parseInt(result[1], 16),
          g: parseInt(result[2], 16),
          b: parseInt(result[3], 16)
        } : null;
      }

    /**
     * Returns a background color for an event, slightly grayed when it is disabled
     * @param {string} mode - mode name for event
     * @param {boolean} isDisabled - true for when event can't be edited
     */
    getColorForMode(mode, isDisabled) {
        let hexCode = this.props.modeData[mode] == null || this.props.modeData[mode].color_code == null ? '#d1d1d1' : this.props.modeData[mode].color_code;
        let rgbCode = this.hexToRgb(hexCode)
        return `rgba(${rgbCode.r},${rgbCode.g},${rgbCode.b},${isDisabled ? '.45' : '1'})`;
    }

    /**
     * Converts minutes to hours + minutes
     * @param {number} minutes - minutes
     */
    getDurationFromMinutes(minutes) {
        let hours = Math.floor(minutes / 60);
        let minutesLeft = minutes % 60;
        return `${hours} hours, ${minutesLeft} minutes`;
    }

    /**
     * Returns a readable time for the event bubble
     */
    getTimes(timeSinceMidnight, isFallBack = false) {
        let startingHours;
        let startingMinutes;
        let yesterdayOrTomorrowText = "";
        if (timeSinceMidnight < 0) {
            if (isFallBack && timeSinceMidnight < -1380) {
                yesterdayOrTomorrowText = "(yesterday)"
                timeSinceMidnight += 60;
                startingHours = Math.floor((1440 + timeSinceMidnight) / 60);
                startingMinutes = (1440 + timeSinceMidnight) % 60;
            } else {
                yesterdayOrTomorrowText = "(yesterday)"
                startingHours = Math.floor((1440 + timeSinceMidnight) / 60);
                startingMinutes = (1440 + timeSinceMidnight) % 60;
            }
            
            
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
     * Returns true if a particular event is selected
     * @param {object} eventInTimeline - the event in question
     */
    isSelected(eventInTimeline) {
        if (typeof(this.props.selectedEvent) != 'undefined' && this.props.selectedEvent != null) {
            if (eventInTimeline.duration == 0) {
                return false;
            }
            if (this.props.selectedEventDayIsToday && this.props.isToday) {
                
                if (this.props.selectedEvent.start_time === eventInTimeline.start_time) {
                    return true;
                }
            } else if (!this.props.selectedEventDayIsToday && !this.props.isToday) { //selectedevent is tomorrow but timeline is today
                if (this.props.selectedEvent.start_time === eventInTimeline.start_time) {
                    return true;
                }
            } else if (this.props.selectedEventDayIsToday && !this.props.isToday) { //selectedevent is today but timeline is tomorrow
                if (isDayDST(true, false, this.props.timezone)) { //fall back yesterday {
                    if (this.props.selectedEvent.start_time === eventInTimeline.start_time + 1500) {
                        return true;
                    }
                } else if (isDayDST(true, true, this.props.timezone)) { //spring forward yesterday {
                    if (this.props.selectedEvent.start_time === eventInTimeline.start_time + 1380) {
                        return true;
                    }
                }
                if (this.props.selectedEvent.start_time - 1440 === eventInTimeline.start_time) {
                    return true;
                }
            } else if (!this.props.selectedEventDayIsToday && this.props.isToday) { //selectedevent is tomorrow but timeline is today
                if (1440 + this.props.selectedEvent.start_time === eventInTimeline.start_time) {
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * Returns true if event has ended/is past the current time
     */
    isDisabled(eventInTimeline) {
        if (this.props.isToday && (eventInTimeline.start_time + eventInTimeline.duration) / this.getProperAmountOfTimeInDay() <= ((this.props.time.getHours() * 60 + this.props.time.getMinutes()) / this.getProperAmountOfTimeInDay())) {
            return true;
        }
        if (eventInTimeline.duration == 0) {
            return true;
        }
        return false;
    }

    /**
     * Returns true if the day selected on the event list is fall back day
     */
    isTodayFallBack() {
        return isDayDST(this.props.isToday, false, this.props.timezone);
    }

    /**
     * Returns 1500 if day is fall back, 1440 otherwise
     */
    getProperAmountOfTimeInDay() {
        if (this.isTodayFallBack()) {
            return 1440 + 60;
        } else {
            return 1440;
        }
    }

    /**
     * Finds event (if the event started "yesterday", it finds the proper start time) and passes it up to scheduler to match it with the server events
     */
    selectEvent(eventData) {
        let actualEvent;
        if (eventData.start_time < 0) {
            this.props.eventsToday.forEach(element => {
                  if (eventData.start_time + 1440 == element.start_time && eventData.mode == element.mode) {
                      actualEvent = element; 
                  }
                });
        }
        this.props.selectEvent(actualEvent == null ? eventData : actualEvent, actualEvent == null ? this.props.isToday : true);
    }

    /**
     * Returns all the buttons in the list
     */
    getEventButtons() {
        const eventsData = this.props.isToday ? this.props.eventsToday : this.props.eventsTomorrow;
        let buttons = [];
        for (let i = 0; i < eventsData.length; i++) {
            let fullTitle = `${eventsData[i].mode}`;
            let includeOutline= this.isSelected(eventsData[i]);
            let adjustedStartTime = eventsData[i].start_time;
            let adjustedDuration = eventsData[i].duration;
            if (isDayDST(this.props.isToday, true, this.props.timezone)) { //spring forward today?
                if (adjustedStartTime >= 120) {
                    adjustedStartTime += 60;
                }
                if ( eventsData[i].start_time < 120 &&  eventsData[i].start_time + adjustedDuration >= 120) {
                    adjustedDuration += 60;
                }
            }
            if (isDayDST(this.props.isToday, false, this.props.timezone)) { //fall back today?
                if (adjustedStartTime >= 120) {
                    adjustedStartTime -= 60;
                }
                if ( eventsData[i].start_time < 120 &&  eventsData[i].start_time + adjustedDuration >= 120) {
                    adjustedDuration -= 60;
                }
            }
            let isDayFallBack = false;
            if (eventsData[i].start_time < 0 && isDayDST(true, false, this.props.timezone)) { //fall back was yesterday
                isDayFallBack = true;
            }
            buttons.push(
                <Button variant = {includeOutline ? "outlined" : "contained"}
                disabled={this.isDisabled(eventsData[i])}
                onClick={() => this.selectEvent(eventsData[i])}
                 style={{ border: `${includeOutline ? "2px solid": "0px solid"}`, margin: '4px', minWidth: '97%', backgroundColor:`${this.getColorForMode(eventsData[i].mode, this.isDisabled(eventsData[i]))}` }}>
                    <div style = {{ width: '100%', display: 'flex', flexDirection: 'column', alignItems: 'center' }}>
                        <Typography>
                            {fullTitle}
                        </Typography>
                        <Typography>
                            {`${this.getTimes(adjustedStartTime, isDayFallBack)} - ${this.getTimes(adjustedStartTime + adjustedDuration, isDayFallBack)}`}
                        </Typography>
                        <Typography variant="caption">
                            {this.getDurationFromMinutes(eventsData[i].duration)}
                        </Typography>
                    </div>
                </Button>
            );
        }
        return buttons;
    }

    getEventAddButton() {
        return(
            <Button disabled={this.props.addEventButtonIsSelected} variant="contained" color="primary" onClick={this.props.addEventButtonSelected()}>
                Add Event
            </Button>
        )
    }

    render() {
        var goToTomorrowButton = (<IconButton
            disabled={!this.props.isToday}
            color="primary"
            aria-label="move right"
            component="span"
            onClick={this.props.changeToTomorrow()}
            size="large">
                                    <KeyboardArrowRightIcon />
                                </IconButton>);
        var goToTodayButton = (<IconButton
            disabled={this.props.isToday}
            color="primary"
            aria-label="move right"
            component="span"
            onClick={this.props.changeToToday()}
            size="large">
                                <KeyboardArrowLeftIcon />
                            </IconButton>);

        return (
            <React.Fragment>
                <div style={{ height: '10%' }}
                >
                    <Grid container style={{ }}>
                        <Grid item xs={3} style={{ display: 'flex', justifyContent: 'center', alignItems: 'center'}}>
                            {goToTodayButton}
                        </Grid>
                        <Grid item xs={6} style={{ display: 'flex', justifyContent: 'center', alignItems: 'center'}}>
                            <Typography style={{ fontWeight: 'bold', fontSize: '16px' }}>
                                {this.props.isToday ? "Today": "Tomorrow"}
                            </Typography>
                        </Grid>
                        <Grid item xs={3} style={{ display: 'flex', justifyContent: 'center', alignItems: 'center'}}>
                            {goToTomorrowButton}
                        </Grid>
                    </Grid>
                    
                </div>
                <div style={{ overflowY: 'scroll', height: '80%', width: '100%' }}>
                    {this.getEventButtons()}
                </div>
                <div style={{ height: '10%', display: 'flex', justifyContent: 'center', alignItems: 'center' }}>
                    {this.getEventAddButton()}
                </div>
            </React.Fragment>
        )
    }
} 

export default withStyles(EventList, STYLES_EVENTLIST);
