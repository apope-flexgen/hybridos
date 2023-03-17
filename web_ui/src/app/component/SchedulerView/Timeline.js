import React from 'react';
import { withStyles } from 'tss-react/mui';
import Typography from '@mui/material/Typography';
import Paper from '@mui/material/Paper';
import Divider from '@mui/material/Divider';
import Chip from '@mui/material/Chip';

import moment from 'moment-timezone';

import { STYLES_TIMELINE } from '../../styles';

import { isDayDST } from '../../../AppFunctions';

const times = ["12:00 AM", "2:00 AM", "4:00 AM", "6:00 AM", "8:00 AM", "10:00 AM", "12:00 PM", "2:00 PM", "4:00 PM", "6:00 PM", "8:00 PM", "10:00 PM", "12:00 AM"];
const timesForFallBack = ["12", "1 ", "1 ", "2 ", "3 ", "4 ", "5 ", "6 ", "7 ", "8 ", "9 ", "10", "11", "12", "1 ", "2 ", "3 ", "4 ", "5 ", "6 ", "7 ", "8 ", "9 ", "10", "11", "12"];

class Timeline extends React.Component {
    constructor(props) {
        super(props);
        this.selectEvent = this.selectEvent.bind(this);

    }

    /**
     * Returns proper coloring for events on timeline.
     * @param {string} mode - mode's name
    */
    getColorForMode(mode) {
        let colorCode = this.props.modeData[mode] != null ? this.props.modeData[mode].color_code : null;
        if (colorCode == null) {
            return '#d1d1d1';
        } else {
            return colorCode;
        }
    }

    /**
     * Returns if an event is selected (determines if outlined in timeline).
     * @param {object} eventInTimeline - event
    */
    isSelected(eventInTimeline) {
        if (typeof(this.props.selectedEvent) != 'undefined' && this.props.selectedEvent != null) {
            if (eventInTimeline.duration == 0) {
                return false;
            }
            if (this.props.selectedEventDayIsToday && this.props.title == "Today") {
                if (this.props.selectedEvent.start_time === eventInTimeline.start_time) {
                    return true;
                }
            } else if (!this.props.selectedEventDayIsToday && this.props.title == "Tomorrow") { //selectedevent is tomorrow but timeline is today
                if (this.props.selectedEvent.start_time === eventInTimeline.start_time) {
                    return true;
                }
            } else if (this.props.selectedEventDayIsToday && this.props.title == "Tomorrow") { //selectedevent is today but timeline is tomorrow
                if (isDayDST(true, false, this.props.timezone)) { //fall back yesterday {
                    if (this.props.selectedEvent.start_time === eventInTimeline.start_time + 1500) {
                        return true;
                    }
                } else if (isDayDST(true, true, this.props.timezone)) { //spring forward yesterday {
                    if (this.props.selectedEvent.start_time === eventInTimeline.start_time + 1380) {
                        return true;
                    }
                }
                if (this.props.selectedEvent.start_time - this.getProperAmountOfTimeInDay() === eventInTimeline.start_time) {
                    return true;
                }
            } else if (!this.props.selectedEventDayIsToday && this.props.title == "Today") { //selectedevent is tomorrow but timeline is today
                if (1440 + this.props.selectedEvent.start_time === eventInTimeline.start_time) {
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * Returns true if event is disabled (reasons: start time has passed timezone time or if duration = 0 which happens on spring forward days for events that get cut off on the hour skip).
     * @param {object} eventInTimeline - event
    */
    isDisabled(eventInTimeline) {
        if (this.props.title == "Today" && (eventInTimeline.start_time + eventInTimeline.duration) / this.getProperAmountOfTimeInDay() <= ((this.props.time.getHours() * 60 + this.props.time.getMinutes()) / this.getProperAmountOfTimeInDay())) {
            return true;
        }
        if (eventInTimeline.duration == 0) {
            return true;
        }
        return false;
    }

    /**
     * Checks if today/tomorrow (depending on which timeline it is) is either spring forward or fall back
    */
    isTodayADSTDay() {
        return isDayDST(this.props.title == "Today", true, this.props.timezone) && isDayDST(this.props.title == "Today", false, this.props.timezone);
    }

    /**
     * Checks if today/tomorrow (depending on which timeline it is) is spring forward
    */
    isTodaySpringForward() {
        return isDayDST(this.props.title == "Today", true, this.props.timezone);
    }

    /**
     * Checks if today/tomorrow (depending on which timeline it is) is fall back
    */
    isTodayFallBack() {
        return isDayDST(this.props.title == "Today", false, this.props.timezone);
    }

    /**
     * Returns 1440 minutes for normal days or 1500 minutes for fall back days
    */
    getProperAmountOfTimeInDay() {
        if (this.isTodayFallBack()) {
            return 1440 + 60;
        } else {
            return 1440;
        }
    }

    /**
     * Returns an array with position 0 being the events that are clickable where they are on the timeline; position 1 as a red box for highlighting DST on the timeline
    */
    getEventChips() {
        let timeInDay = this.getProperAmountOfTimeInDay();
        const eventsData = this.props.title == "Today" ? this.props.eventsToday : this.props.eventsTomorrow;
        let boxes = [];
        let labels = [];
        if (this.isTodaySpringForward()) {
            const theTime = new Date();    
        const convertTime = moment(theTime).tz(this.props.timezone).format("YYYY-MM-DD HH:mm:ss");
        const result = new Date(convertTime);
            if (!(this.props.title == "Today")) {
                result.setDate(result.getDate() + 1);
            }

            let width = `${(60) / timeInDay * 100}%`;
            let positionOnPage = `${(1 * 60 + 59) / timeInDay * 100}%`;
            labels.push(
                <Typography align="center" style={{
                            backgroundColor: 'transparent',
                            pointerEvents: 'none',
                            width: {width},
                            height: '50%',
                            left: `${positionOnPage}`,
                            top: '25%',
                            bottom: '25%',
                            position: 'absolute'
             }}>DST</Typography>
            );
            labels.push(<div style={{ pointerEvents: 'none', position: 'absolute', height: '100%', backgroundColor: '#ff0000', width: "4.1666%", left: `${positionOnPage}` }} />);
        }
        for (let i = 0; i < eventsData.length; i++) {
            let adjustedStartTime = eventsData[i].start_time;
            let adjustedDuration = eventsData[i].duration;
            if (this.isTodaySpringForward()) {
                if (adjustedStartTime >= 120) {
                    adjustedStartTime += 60;
                }
                if ( eventsData[i].start_time < 120 &&  eventsData[i].start_time + adjustedDuration >= 120) {
                    adjustedDuration += 60;
                }
            }
            let positionOnPage = `${adjustedStartTime / timeInDay * 100}%`;
            let includeOutline= this.isSelected(eventsData[i]);
            let width = `${(adjustedDuration) / timeInDay * 100}%`;

            boxes.push(
                <Chip style={{ backgroundColor: `${this.getColorForMode(eventsData[i].mode)}`,

                               border: `${includeOutline ? "1px solid": "0px solid"}`,
                               width: `${width}`,
                               height: '50%',
                               left: `${positionOnPage}`,
                               top: '25%',
                               bottom: '25%',
                               position: 'absolute'
                            }}
                            onClick={() => this.selectEvent(eventsData[i])}
                            disabled={this.isDisabled(eventsData[i])}
                
            />
            );
        }
        return [boxes, labels];
    }

    /**
     * Selects event and tells Scheduler.js which event is selected
     * @param {object} eventData - event selected
    */
    selectEvent(eventData) {
        let actualEvent;
        if (eventData.start_time < 0) {
            this.props.eventsToday.forEach(element => {
                  if (eventData.start_time + 1440 == element.start_time && eventData.mode_selection == element.mode_selection) {
                      actualEvent = element; 
                  }
                });
        }
        
        this.props.selectEvent(actualEvent == null ? eventData : actualEvent, actualEvent == null ? this.props.title == "Today" : true);
    }

    /**
     * Returns the moving black bar across the today timeline
    */
    getCurrentTimeBar() {
        let hours = this.props.time.getHours()
        return (
            <div style={{ borderLeft: "3px solid black", height: '100%', position: 'absolute', left: `${(hours * 60 + this.props.time.getMinutes())/1440*100}%`, marginLeft: '-3px', top: '0' }}/>
        );
    }

    render() {
        let dividers = [];
        let numDividers = 24;
        if (this.isTodayFallBack()) {
            numDividers += 1;
        }

        for (let i = 0; i < numDividers + 1; i++) {
            if (i == 0 || i == numDividers) {
                dividers.push(
                    (<Divider style={{ opacity: '0%' }}orientation="vertical" ></Divider>)
                )
            } else {
                dividers.push(
                    (<Divider style={{ borderColor: 'black', opacity: `${i % 2 == 0 ? '50%' : '10%'}` }} orientation="vertical"></Divider>)
                );
            } 
        }
        let title = this.props.title;
        let isTodayFallBack = false;
        if (this.isTodaySpringForward()) {
            title += " (Spring Forward Today)"
        }
        if (this.isTodayFallBack()) {
            isTodayFallBack = true;
            title += " (Fall Back Today)"
        }
        let eventChipsReturn = this.getEventChips();
        let properWidth = !isTodayFallBack ? '95%' : '99.666%';
        return (
            <div style={{ width: '100%', display: 'flex', flexDirection: 'column', alignItems: 'center' }}>
                <Typography style={{ fontWeight: 'bold', fontSize: '24px', marginBottom: '4px' }}>
                    {title}
                </Typography>
                {isTodayFallBack && 
                     <div style={{ width: '101%', display: 'flex', alignItems: 'center', justifyContent: 'space-between'}}>
                     {timesForFallBack.map((time) => (
                         <Typography style={{ fontSize: '16px' }}>{time}</Typography>))
                     }
                 </div>}
                 {!isTodayFallBack && 
                     <div style={{ width: '100%', display: 'flex', alignItems: 'center', justifyContent: 'space-between'}}>
                     {times.map((time) => (
                        <Typography style={{ fontSize: '16px' }}>{time}</Typography>
                    ))}
                 </div>}

                {/* <div style={{ width: '102%', display: 'flex', alignItems: 'center', justifyContent: 'space-between'}}>
                    {isTodayFallBack && timesForFallBack.map((time) => (
                        <Typography style={{ fontSize: '14px' }}>{time}</Typography>
                    ))}
                    {!isTodayFallBack && times.map((time) => (
                        <Typography style={{ fontSize: '16px' }}>{time}</Typography>
                    ))}
                </div> */}
                <Paper style={{ overflowX: 'hidden', width: `${properWidth}`, height: '60px', justifyContent: 'center', margin: 'auto', position: 'relative', borderColor: 'rgba(0, 0, 0, 0.5)'}} variant="outlined">
                    <div style={{ position: 'absolute', width: '100%', height: '100%', display: 'flex', justifyContent: 'space-between' }}>
                        {dividers}
                    </div>
                    {this.props.title == "Today" && this.getCurrentTimeBar()}
                    {eventChipsReturn[1][1]}
                    {eventChipsReturn[0]}
                    {eventChipsReturn[1][0]}
                </Paper>
            </div>
        )
    }
} 

export default withStyles(Timeline, STYLES_TIMELINE);
