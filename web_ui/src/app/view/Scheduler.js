import React from 'react';
import { withStyles } from 'tss-react/mui';

import moment from 'moment-timezone';

import Paper from '@mui/material/Paper';

import FormControl from '@mui/material/FormControl';
import MenuItem from '@mui/material/MenuItem';
import Select from '@mui/material/Select';
import InputLabel from '@mui/material/InputLabel';
import FormHelperText from '@mui/material/FormHelperText';
import ErrorIcon from '@mui/icons-material/Error';

import Typography from '@mui/material/Typography';

import equal from 'deep-equal'

import Box from '@mui/material/Box';

import { STYLES_SCHEDULER } from '../styles';
import Timeline from '../component/SchedulerView/Timeline';
import EventList from '../component/SchedulerView/EventList';
import FormList from '../component/SchedulerView/FormList';
import { isDayDST } from '../../AppFunctions';

import {
  getDataForURI,  setOrPutDataForURI
} from '../../AppConfig';

import {
  siteConfiguration,
} from '../../AppAuth';

import LoadingHOC from '../component/LoadingHOC';
import { socket_scheduler } from '../../AppAuth';
  
class Scheduler extends React.Component {

    constructor(props) {
      super(props);
      this.state = {
          selectedEvent: null,
          selectedEventDayIsToday: false,
          isTodaySelected: true, //false for tomorrow
          addEventButtonSelected: false,
          eventsToday: [],
          eventsTomorrow: [],
          eventsDayAfterTomorrow: [],
          siteData: [],
          connectionType: "",
          siteSelected: "",
          siteConnection: {},
          sitesLoading: true,
          modeData: {},
          time: new Date(new Date().toLocaleString("en-US", {timeZone: Intl.DateTimeFormat().resolvedOptions().timeZone})),
          modesLoading: true,
          eventsLoading: true,
          timezones: {},
          timezone: 'America/Chicago',
          connectedLoading: true,
          timezonesLoading: true,
          socket: socket_scheduler,

          eventDataFromBackEnd: null,
      }

      this._isMounted = false;

      this.changeToToday = this.changeToToday.bind(this);
      this.handleDropDown = this.handleDropDown.bind(this);
      this.changeToTomorrow = this.changeToTomorrow.bind(this);
      this.selectEvent = this.selectEvent.bind(this);
      this.isOverlappingStartTime = this.isOverlappingStartTime.bind(this);
      this.isOverlappingDuration = this.isOverlappingDuration.bind(this)
      this.addEventButtonSelected = this.addEventButtonSelected.bind(this);
      this.cancelAddEvent = this.cancelAddEvent.bind(this);
      this.submitEvent = this.submitEvent.bind(this);
      this.deleteEvent = this.deleteEvent.bind(this);

    }

    /**
     * Sets up all the sockets and starts an interval to update the time every 10 seconds
     */
    componentDidMount() {
      this.state.socket.on('/scheduler/modes', (data) => {
        this.updateModeStateData(JSON.parse(data));
      });
      this.state.socket.on('/scheduler/events', (data) => {
        this.updateEventStateData(JSON.parse(data));
      });
      this.state.socket.on('/scheduler/configuration', (data) => {
        this.updateSiteData(JSON.parse(data));
      });
      this.state.socket.on('/scheduler/connected', (data) => {
        this.updateSiteConnection(JSON.parse(data));
      });
      this.state.socket.on('/scheduler/timezones', (data) => {
        this.updateTimezones(JSON.parse(data));
      });
      // this.state.socket.on('/scheduler', (data) => {
      //   this.siteConnection(data)
      // })
      // this.state.socket.onmessage = this.siteConnection;
      this.interval = setInterval(() => this.updateTime(), 10 * 1000);
      this._isMounted = true;
      setTimeout(() => {
          this._isMounted && this.fetchData();
      }, 1000);
    }

    /**
     * Updates timezone map
     * @param {object} data - looks like {siteName: timezone, ...}
     */
    updateTimezones(data) {
      this.setState({
        timezones: data,
        timezonesLoading: false
      }, () => this.updateTime())
    }

    /**
     * Updates connected map (is site connected or not) and if the currently selected site was not connected but then connected, it fetches that site's data again
     * @param {object} data - looks like {siteName: boolean, ...}
     */
    updateSiteConnection(event) {
      // event = JSON.parse(event)
      let copyWithChangesFromEvent = JSON.parse(JSON.stringify(this.state.siteConnection));
      let copyNoChanges = JSON.parse(JSON.stringify(this.state.siteConnection));

      let keys = Object.keys(event);
      let reset = false;
      for (let i = 0; i < keys.length; i++) {
        copyWithChangesFromEvent[keys[i]] = event[keys[i]];
        if (this.state.siteSelected == keys[i] && event[this.state.siteSelected] == true && copyNoChanges[keys[i]] == false) {
          reset = true;
        }
      }

      if (this.state.siteSelected)

      this.setState({
        selectedEvent: reset ? null : this.state.selectedEvent,
          selectedEventDayIsToday: reset ? false : this.state.selectedEventDayIsToday,
          isTodaySelected: reset ? true : this.state.isTodaySelected,
          addEventButtonSelected: reset ? false : this.state.addEventButtonSelected,
        eventsLoading: reset ? true : this.state.eventsLoading,
        siteConnection: copyWithChangesFromEvent,
        connectedLoading: false
      }, () => {
        if (reset) {
          this.fetchData()
        }
      })
      
    }

    /**
     * Updates the time with the proper timezone time and if an event is selected that has now ended, it forces the event to be unselected
     */
    updateTime() {
      let defaultTimeZone = Intl.DateTimeFormat().resolvedOptions().timeZone;
      if (this.state.siteSelected == "") {
        return;
      }

      const theTime = new Date();    

      if (moment(theTime).tz(this.state.timezones[this.state.siteSelected]) == null) { return; }
      const convertTime = moment(theTime).tz(this.state.timezones[this.state.siteSelected]).format("YYYY-MM-DD HH:mm:ss");
      const newTime = new Date(convertTime);
      
      if (this.state.selectedEvent != null && this.state.selectedEventDayIsToday && (this.state.selectedEvent.start_time + this.state.selectedEvent.duration) / this.getProperAmountOfTimeInDay() < ((this.state.time.getHours() * 60 + this.state.time.getMinutes()) / this.getProperAmountOfTimeInDay())) {
        this.setState({
          time: newTime,
          timezone: this.state.timezones[this.state.siteSelected],
          // time: new Date(new Date().toLocaleString("en-US", {timeZone: this.state.timezones[this.state.siteSelected]})),
          selectedEvent: null
        });
      } else {
        this.setState({
          time: newTime,
          timezone: this.state.timezones[this.state.siteSelected],
          // time: new Date(new Date().toLocaleString("en-US", {timeZone: this.state.timezones[this.state.siteSelected]})),
        });
      }
    }
    
    /**
     * returns true if day is fall back day
     * @param {boolean} today - check today if true, tomorrow if false
     */
    isTodayFallBack(today) {
      return isDayDST(today, false, this.state.timezone);
    }

    /**
     * returns true if day is spring forward day
     * @param {boolean} today - check today if true, tomorrow if false
     */
    isTodaySpringForward(today) {
      return isDayDST(today, true, this.state.timezone);
    }


    /**
     * Closes sockets and the updating time interval when user leaves page
     */
    componentWillUnmount() {
        clearInterval()
        this.state.socket.off('/scheduler/events');
        this.state.socket.off('/scheduler/modes');
        this.state.socket.off('/scheduler/configuration');
        this.state.socket.off('/scheduler/connection');
        this.state.socket.off('/scheduler/timezones');

        this._isMounted = false;
    }

    /**
     * Fetches all the initial data (configuration (fleetmanager or sitecontroller), modes, events, if sites are connected, site's timezones)
     */
    fetchData() {
      getDataForURI('scheduler/configuration')
      .then((response) => {
        if (response.ok) {
          getDataForURI('scheduler/modes')
            .then((response) => {
                if (response.ok) {
                    getDataForURI('scheduler/events')
                    .then((response) => {
                        if (response.ok) {
                            getDataForURI('scheduler/connected')
                            .then((response) => {
                                if (response.ok) {
                                  getDataForURI('scheduler/timezones')
                                  .then((response) => {
                                      if (response.ok) {
                                          return response.json();
                                      }
                                      throw new Error(`${response.statusText} : Cannot get SCHEDULER TIMEZONES`);
                                  })
                                  .catch((error) => this.setState({ error }));
                                }
                                throw new Error(`${response.statusText} : Cannot get SCHEDULER CONNECTED`);
                            })
                            .catch((error) => this.setState({ error }));
                        }
                        throw new Error(`${response.statusText} : Cannot get SCHEDULER EVENTS`);
                    })
                    .catch((error) => this.setState({ error }));
                }
                throw new Error(`${response.statusText} : Cannot get SCHEDULER MODES`);
            })
            .catch((error) => this.setState({ error }));

        }
        throw new Error(`${response.statusText} : Cannot get SCHEDULER CONFIG`);
      })
      .catch((error) => this.setState({ error, isLoading: false }));

      // getDataForURI('scheduler/modes')
      //       .then((response) => {
      //           if (response.ok) {
      //               getDataForURI('scheduler/events')
      //               .then((response) => {
      //                   if (response.ok) {
      //                       return response.json();
      //                   }
      //                   throw new Error(`${response.statusText} : Cannot get SCHEDULER EVENTS`);
      //               })
      //               .catch((error) => this.setState({ error }));
      //           }
      //           throw new Error(`${response.statusText} : Cannot get SCHEDULER MODES`);
      //       })
      //       .catch((error) => this.setState({ error }));
      
      // let siteKeys =  Object.keys(this.state.siteConnection);
      // for (let i = 0; i < siteKeys.length; i++) {
      //   // this.state.socket.off('/scheduler/' + siteKeys[i] + '/connected');
      //   getDataForURI('/scheduler/' + siteKeys[i] + '/connected')
      //   .then((response) => {
      //     if (response.ok) {
  
      //       return response.json();
  
      //     }
      //     throw new Error(`${response.statusText} : Cannot get SCHEDULER CONNECTION to ` + siteKeys[i]);
      //   })
      //   .catch((error) => this.setState({ error, isLoading: false }));
      // }
     
      
    }

    /**
     * receieves site info and also sets up site connection map
     * @param {object} data - example: { "connection": "SC", "siteId": "san_diego", "siteName": "San Diego", "siteControllerPort": "9000" }
     */
    updateSiteData(data) {
      let siteKeys =  Object.keys(this.state.siteConnection);
      let sitesList = [];
      let siteConnectionData = JSON.parse(JSON.stringify(this.state.siteConnection));
      if (data.connection == "FM") {
        for (let i = 0; i < data.sites.length; i++) {
          sitesList.push([data.sites[i]['siteName'], data.sites[i]['siteId']]);
        }
      } else {
        sitesList.push([data['siteName'], data['siteId']]);
      }
      sitesList.sort();

      siteKeys =  Object.keys(siteConnectionData);
      
      // this.getSiteConnections('9')
      let siteSelected = this.state.siteSelected == "" ? sitesList[0][1] : this.state.siteSelected;
      let newSiteConnectionObject = {};
      newSiteConnectionObject[siteSelected] = true;
      this.setState({
        connectionType: data.connection,
        connectedLoading: data.connection == "FM" ? this.state.connectedLoading : false,
        siteConnection: data.connection == "FM" ? siteConnectionData : newSiteConnectionObject,
        siteData: sitesList,
        siteSelected: siteSelected,
        sitesLoading: false

      })
    }

    /**
     * receieves modes info
     * @param {object} data - example: "charging": { "color_code": "0xFF5532", variables: [ { "id": "charge_cmd", "name": "Manual Charge command", "type": "Float", "unit": "kW", "uri": "/features/active_power/manual_ess_kW_cmd", "value": 5000 } ], constants: [ { "id": "charge_feature", "type": "Integer", "name": "Feature Mode Selection", "uri":"/features/active_power/features_kW_Mode_cmd", "value":2 } ] }
     */
    updateModeStateData(data) {
      this.setState({
        modeData: data.modes,
        modesLoading: false,
        selectedEvent: null,
          selectedEventDayIsToday: null,
          addEventButtonSelected: false,
        // eventsLoading: reset ? true : this.state.eventsLoading,
        // siteConnection: copyWithChangesFromEvent,
        // connectedLoading: false
      })
    }

    /**
     * receieves schedule for all sites as a map
     * @param {object} data - map w main key being schedule, then keys for siteIds, then in each of those there is an array with 2 indices (today and tomorrow) with an array in that with events like: { "start_time": 555, "duration": 90, "Mode": "charging", "variables": { "charge_cmd": 5000.0 } }
     */
    updateEventStateData(data) {
      if (this.state.siteSelected == "" || this.state.siteSelected == null) {
        return;
      }
        let todayData;
        let tomorrowData;
        if (Object.keys(data.schedule).length == 0) {
          todayData = [];
          tomorrowData = [];
        } else {
          todayData = data.schedule[this.state.siteSelected][0];
          tomorrowData = data.schedule[this.state.siteSelected][1];
        }
      //changes to add duplicate events (add event to tomorrow array if an event in today array goes into tomorrow)
      let needToAddDuplicate = true;
      for (let i = 0; i < tomorrowData.length; i++) {
        if (tomorrowData[i].start_time < 0) {
          needToAddDuplicate = false;
          break;
        }
      }
      if (needToAddDuplicate) {
        for (let i = 0; i < todayData.length; i++) {
          if (todayData[i].start_time + todayData[i].duration > this.getProperAmountOfTimeInDay()) {
            let duplicateData = JSON.parse(JSON.stringify(todayData[i]));
            duplicateData.start_time = duplicateData.start_time - this.getProperAmountOfTimeInDay();
            tomorrowData.push(duplicateData);
            break;
          }
        }
      }

      //sort both with sortEvents
      if (todayData != null) {
        this.sortEvents(todayData);
      }
      if (tomorrowData != null) {
        this.sortEvents(tomorrowData);
      }

      //dbi doesn't save variables key when the mode has no vars attached
      for (let i = 0; i < todayData.length; i++) {
        if (!("variables" in todayData[i])) {
          todayData[i]["variables"] = {};
        }
      }
      for (let i = 0; i < tomorrowData.length; i++) {
        if (!("variables" in tomorrowData[i])) {
          tomorrowData[i]["variables"] = {};
        }
      }
    
      this.setState({
        eventsToday: todayData == null ? [] : todayData,
        eventsTomorrow: tomorrowData == null ? [] : tomorrowData,
        eventsLoading: false,
        eventDataFromBackEnd: JSON.parse(JSON.stringify([todayData, tomorrowData]))
      });
    }

     /**
     * sorts events by start time when new event is added
     * @param {array} listOfEvents - events to be sorted
     */
    sortEvents(listOfEvents) {
      listOfEvents.sort((a, b) => (a.start_time > b.start_time) ? 1 : -1)
    }

    /**
     * finds event and selects (must find it because sometimes event has start time in the negatives which means you have to check the day before)
     * @param {object} scheduledEvent - event that may have a negative start time; this event finds the positive start time version held by the server
     * @param {object} isToday - is event today or tomorrow
     */
    selectEvent(scheduledEvent, isToday) {
      // let isFallBack = isDayDST(true, false);
      let actualEvent;
        if (scheduledEvent.start_time < 0) {
            this.state.eventsToday.forEach(element => {
                  if (scheduledEvent.start_time + (this.getProperAmountOfTimeInDay(true)) == element.start_time && scheduledEvent.mode == element.mode && element.duration == scheduledEvent.duration) {
                      actualEvent = element; 
                  }
                });
        }
      this.setState({
        selectedEvent: actualEvent == null ? scheduledEvent : actualEvent,
        selectedEventDayIsToday: actualEvent == null ? isToday : true,
      }, () => this.updateTime());
    }

    /**
     * Change isTodaySelected to yes (for Event List and the form when creating a new event)
     */
    changeToToday() {
      this.setState({
        isTodaySelected: true
      });
    }

    /**
     * Change isTodaySelected to no (for Event List and the form when creating a new event)
     */
    changeToTomorrow() {
      this.setState({
        isTodaySelected: false
      });
    }

    /**
     * Unselects any event and initiates form opening
     */
    addEventButtonSelected() {
      this.setState({
        selectedEvent: null,
        addEventButtonSelected: true
      });
    }

    /**
     * Change isTodaySelected to yes (for Event List)
     */
    cancelAddEvent() {
      this.setState({
        selectedEvent: null,
        addEventButtonSelected: false
      });
    }

    /**
     * Deletes the event that is held by the state variable selectedEvent
     * @param submittedEvent - if not null, after deleting the selectedEvent, this function will cause this event to be created
     * @param selectedEventDayIsToday - if not null, this is paired with the submitted event to create a new event
     */
    deleteEvent(submittedEvent = null, selectedEventDayIsToday = null) {
      let newTodayEventList;
      let newTomorrowEventList;
      let copiedObject = JSON.parse(JSON.stringify(this.state.selectedEvent));
        
      if (this.state.selectedEventDayIsToday) {
        newTodayEventList = this.state.eventsToday.filter(element => element != this.state.selectedEvent);
        copiedObject.start_time = this.state.selectedEvent.start_time - this.getProperAmountOfTimeInDay(true);
        newTomorrowEventList = this.state.eventsTomorrow.filter(element => element.start_time != copiedObject.start_time || element.mode != copiedObject.mode);
      } else {
        newTomorrowEventList = this.state.eventsTomorrow.filter(element => element != this.state.selectedEvent);
        copiedObject.start_time = this.state.selectedEvent.start_time + this.getProperAmountOfTimeInDay();
        newTodayEventList = this.state.eventsToday.filter(element => element.start_time != copiedObject.start_time || element.mode != copiedObject.mode);
      }
      if (submittedEvent == null) {
        this.setState({
          eventsToday: newTodayEventList,
          eventsTomorrow: newTomorrowEventList,
          selectedEvent: null,
        });
        let sentBackObject = {};
        let eventIsToday = true;
        sentBackObject['events'] = eventIsToday ? newTodayEventList : newTomorrowEventList;
        let properDay = eventIsToday ? 'day_0' : 'day_1';
        this.sendToBackEnd(sentBackObject, properDay);
        sentBackObject['events'] = !eventIsToday ? newTodayEventList : newTomorrowEventList;
        this.sendToBackEnd(sentBackObject, properDay == "day_0" ? "day_1" : "day_0");
      } else {
        this.setState({
          eventsToday: newTodayEventList,
          eventsTomorrow: newTomorrowEventList,
          selectedEvent: null,
        }, () => {
          this.continueEditing(submittedEvent, selectedEventDayIsToday)});
      }
      
    }

    /**
     * Checks if the current start time is within any other event's span; skips over selectedEvent if there is one since then you're editing and the start time can be anywhere in the currently selected event's range
     */
    isOverlappingStartTime(startTime) {
        let time = startTime.split(':');
        let hours = parseInt(time[0]);
        let newStartTime = (hours*60)+parseInt(time[1]);
        let whichDay = this.state.selectedEvent == null ? this.state.isTodaySelected : this.state.selectedEventDayIsToday;
        if (isDayDST(whichDay, true, this.state.timezone) && newStartTime >= 120) { //spring forward
          newStartTime -= 60;
        }
        if (isDayDST(whichDay, false, this.state.timezone) && newStartTime >= 120) { //fall back
          newStartTime += 60;
        }
        let whichList;
        if (this.state.selectedEvent != null) {
          whichList = this.state.selectedEventDayIsToday ? this.state.eventsToday : this.state.eventsTomorrow
        } else {
          whichList = this.state.isTodaySelected ? this.state.eventsToday : this.state.eventsTomorrow;

        }
        
        for (let i = 0; i < whichList.length; i++) {
            if (this.state.selectedEvent != null && this.state.selectedEvent == whichList[i]) {
              continue;
            }
            if (whichList[i].duration == 0) {
              continue;
            }
            if (whichList[i].start_time > newStartTime) {
              continue;
            } else if (newStartTime > whichList[i].start_time && newStartTime >= whichList[i].start_time + whichList[i].duration) {
              continue;
            } else {
              return true;
            }
        }

        return false;
    }

    /**
     * Checks if the current start time + duration is within any other event's span; skips over selectedEvent if there is one since then you're editing and the start time + duration can be anywhere in the currently selected event's range
     */
    isOverlappingDuration(startTime, duration) {
      let time = startTime.split(':');
      let hours = parseInt(time[0]);
      let newStartTime = (hours*60)+parseInt(time[1]);
      let whichDay = this.state.selectedEvent == null ? this.state.isTodaySelected : this.state.selectedEventDayIsToday;
      let newDuration = parseInt(duration);
        if (isDayDST(whichDay, true, this.state.timezone) && newStartTime >= 120 ) { //spring forward
          newStartTime -= 60;
        }
        if (isDayDST(whichDay, false, this.state.timezone) && newStartTime >= 120) { //fall back
          newStartTime += 60;
        }


      let whichList;
      if (this.state.selectedEvent != null) {
        whichList = this.state.selectedEventDayIsToday ? this.state.eventsToday : this.state.eventsTomorrow
      } else {
        whichList = this.state.isTodaySelected ? this.state.eventsToday : this.state.eventsTomorrow;

      }
      for (let i = 0; i < whichList.length; i++) {
          if (this.state.selectedEvent != null && this.state.selectedEvent == whichList[i]) {
            continue;
          }
          if (newStartTime < whichList[i].start_time && newStartTime + newDuration > whichList[i].start_time) {
            return true;
          } else {
            continue;
          }
      }
      if ((this.state.selectedEvent != null && this.state.selectedEventDayIsToday) || (this.state.selectedEvent == null && this.state.isTodaySelected)) {
        for (let i = 0; i < this.state.eventsTomorrow.length; i++) {
          if (this.state.selectedEvent != null && this.state.selectedEvent.start_time == this.state.eventsTomorrow[i].start_time + this.getProperAmountOfTimeInDay()) {
            continue;
          }

          if (newStartTime - this.getProperAmountOfTimeInDay() <  this.state.eventsTomorrow[i].start_time && newStartTime - this.getProperAmountOfTimeInDay() + newDuration >  this.state.eventsTomorrow[i].start_time) {
            return true;
          } else {
            continue;
          }
        }
      }
      return false;
  }

    /**
     * Called on by deleteEvent after event has been deleted to initiate a new event creation
     */
    continueEditing(submittedEvent, selectedEventDayIsToday) {
      this.submitEvent(submittedEvent, selectedEventDayIsToday);
    }

    /**
     * When an event is edited, the old event must be deleted, and the delete function manages recreating the submittedEvent
     */
    editEvent(submittedEvent, newStartTime, selectedEventDayIsToday) {
        this.deleteEvent(submittedEvent, selectedEventDayIsToday);
        return;
    }

    /**
     * Returns 1500 for fall back, 1380 for spring forward, 1440 for normal
     * @param {boolean} isToday - null if something is selected so we should use that event's day, true for looking at today, false for tomorrow
     */
    getProperAmountOfTimeInDay(isToday = null) {
      let whichDay = this.state.selectedEvent == null ? this.state.isTodaySelected : this.state.selectedEventDayIsToday;
      if (isToday != null) {
        whichDay = isToday;
      }
      if (isDayDST(whichDay, true, this.state.timezone)) {
        return 1440 - 60;
      }
      if (isDayDST(whichDay, false, this.state.timezone)) {
          return 1440 + 60;
      } else {
          return 1440;
      }
  }

    /**
     * Creates a new event. If an event spans into the next day it creates 2 events, one for the today list and one for tomorrow). Only the day where event starts is actually saved to the back end.
     */
    submitEvent(submittedEvent, eventIsToday = this.state.isTodaySelected) {
      let time = submittedEvent.start_time.split(':');
      let hours = parseInt(time[0]);
      let isFallBack = isDayDST(eventIsToday, false, this.state.timezone);
      
      let newStartTime = (hours*60)+parseInt(time[1]);
      if (isDayDST(eventIsToday, true, this.state.timezone) && newStartTime >= 120) {
        newStartTime -= 60;
      }
      if (isFallBack && newStartTime >= 120) {
        newStartTime += 60;
      }
      if (this.state.selectedEvent != null) {
        this.editEvent(submittedEvent, newStartTime, this.state.selectedEventDayIsToday);
      } else {
        let newEventsToday = this.state.eventsToday;
        let newEventsTomorrow = this.state.eventsTomorrow;
        if (eventIsToday) {
          newEventsToday = this.state.eventsToday.concat({
              start_time: newStartTime,
              duration: parseInt(submittedEvent.duration),
              mode: submittedEvent.mode,
              variables: submittedEvent.variables
          });
          if (newStartTime + parseInt(submittedEvent.duration) >= this.getProperAmountOfTimeInDay(true)) {
            newEventsTomorrow = this.state.eventsTomorrow.concat({
              start_time: newStartTime - (this.getProperAmountOfTimeInDay(true)),
              duration: parseInt(submittedEvent.duration),
              mode: submittedEvent.mode,
              variables: submittedEvent.variables
          });
          }
        } else {
          newEventsTomorrow = this.state.eventsTomorrow.concat({
              start_time: newStartTime,
              duration: parseInt(submittedEvent.duration),
              mode: submittedEvent.mode,
              variables: submittedEvent.variables
          });
          this.setState({
            eventsTomorrow: newEventsTomorrow,
            addEventButtonSelected: false
          });
        }
        this.sortEvents(newEventsToday);
        this.sortEvents(newEventsTomorrow);

        let sentBackObject = {};
        sentBackObject['events'] = eventIsToday ? newEventsToday : newEventsTomorrow;
        let properDay = eventIsToday ? 'day_0' : 'day_1';
        this.sendToBackEnd(sentBackObject, properDay);
        sentBackObject['events'] = !eventIsToday ? newEventsToday : newEventsTomorrow;
        this.sendToBackEnd(sentBackObject, properDay == "day_0" ? "day_1" : "day_0");
        
        this.setState({
          eventsToday: newEventsToday,
          eventsTomorrow: newEventsTomorrow,
          addEventButtonSelected: false
        });    
      }
      
    }

    /**
     * Sends events to backend; also removes any negative start time events since that is only necessary for UI
     * @param {object} sentBackObject - has a key events with a value the array of the day to send back
     * @param {string} day - we send to either day_0 or day_1 but not both when an event is changed
     */
    sendToBackEnd(sentBackObject, day) {
      //changes to get rid of duplicate events (if day is tomorrow, delete any events with negative time)
      sentBackObject = JSON.parse(JSON.stringify(sentBackObject))
      
      if (day == "day_1") {
        let array = sentBackObject.events;
        for (let i = 0; i < array.length; i++) {
          if (array[i].start_time < 0) {
            let removeThisStartTime = array[i].start_time;
            sentBackObject.events = array.filter(element => element.start_time != removeThisStartTime);
            break;
          }
        }
      }

      let sendToBackEnd = false;
      if (day == "day_0") {
        
        if (!equal(this.state.eventDataFromBackEnd[0],(sentBackObject.events))) {
          let newEventStateData = JSON.parse(JSON.stringify(this.state.eventDataFromBackEnd));
          if (this.state.eventDataFromBackEnd[0].length == (sentBackObject.events).length) {
              for (let i = 0; i < this.state.eventDataFromBackEnd[0].length; i++) {
                  let thisEventIsSomewhere = false;
                  for (let j = 0; j < sentBackObject.events.length; j++) {
                      if (equal(this.state.eventDataFromBackEnd[0][i], sentBackObject.events[j])) {
                          thisEventIsSomewhere = true;
                      }
                  }
                  if (thisEventIsSomewhere == false) {
                      sendToBackEnd = true;
                      break;
                  }
              }
              for (let i = 0; i < sentBackObject.events.length; i++) {
                  let thisEventIsSomewhere = false;
                  for (let j = 0; j < this.state.eventDataFromBackEnd[0].length; j++) {
                      if (equal(this.state.eventDataFromBackEnd[0][j], sentBackObject.events[i])) {
                        thisEventIsSomewhere = true;
                      }
                  }
                  if (thisEventIsSomewhere == false) {
                      sendToBackEnd = true;
                      break;
                  }
              }
          } else {
              sendToBackEnd = true;
          }

          newEventStateData[0] = sentBackObject.events;
          this.setState({
            eventDataFromBackEnd: newEventStateData
          })
        }
      } else {
        if (!equal(this.state.eventDataFromBackEnd[1],(sentBackObject.events))) {
          let newEventStateData = JSON.parse(JSON.stringify(this.state.eventDataFromBackEnd));
          if (this.state.eventDataFromBackEnd[1].length == (sentBackObject.events).length) {
              for (let i = 0; i < this.state.eventDataFromBackEnd[1].length; i++) {
                  let thisEventIsSomewhere = false;
                  for (let j = 0; j < sentBackObject.events.length; j++) {
                      if (equal(this.state.eventDataFromBackEnd[1][i], sentBackObject.events[j])) {
                          thisEventIsSomewhere = true;
                      }
                  }
                  if (thisEventIsSomewhere == false) {
                      sendToBackEnd = true;
                      break;
                  }
              }
              for (let i = 0; i < sentBackObject.events.length; i++) {
                  let thisEventIsSomewhere = false;
                  for (let j = 0; j < this.state.eventDataFromBackEnd[1].length; j++) {
                      if (equal(this.state.eventDataFromBackEnd[1][j], sentBackObject.events[i])) {
                        thisEventIsSomewhere = true;
                      }
                  }
                  if (thisEventIsSomewhere == false) {
                      sendToBackEnd = true;
                      break;
                  }
              }
          } else {
              sendToBackEnd = true;
          }

          newEventStateData[1] = sentBackObject.events;
          this.setState({
            eventDataFromBackEnd: newEventStateData
          })


        }
      }

      if (sendToBackEnd) {
        setOrPutDataForURI('scheduler/' + this.state.siteSelected + '/' + day, JSON.stringify(sentBackObject), 'POST')
              .then((response) => {
                  if (response.ok) {
                      return response.json();
                  }
                  throw new Error(`${response.statusText} : Cannot updatePropertyValue() for: ${response.url}`);
              })
              .catch((error) => {
                  throw new Error(`InputFieldControl/doUpdatePropertyValue error: ${error}`);
              });
            }  
    }

    handleDropDown = (event) => {

      this.setState({
        eventsLoading: true,
        siteSelected: event.target.value,
          // input: newInput
      }, () => this.fetchData());
  }
    

    render() {
      let displaySchedulerNeedsConfig = (!this.state.modesLoading && !this.state.sitesLoading) && this.state.eventsLoading && (this.state.siteSelected == "" || this.state.siteSelected == null);
      let displaySpinCircle = (this.state.modesLoading || this.state.eventsLoading || this.state.sitesLoading || this.state.connectedLoading || this.state.timezonesLoading) && !((!this.state.modesLoading && !this.state.sitesLoading) && this.state.eventsLoading && (this.state.siteSelected == "" || this.state.siteSelected == null));
      this.props.setLoading(displaySpinCircle)
      //  && !displaySchedulerNeedsConfig && !displaySpinCircle) && <>


     return (
          <>
          {displaySchedulerNeedsConfig &&
            <Paper style = {{width: '95%', alignItems: 'center', justifyContent: 'center', height: '75px', display: 'flex'}}><Typography variant='h6'>Scheduler is not fully configured. Contact an administrator to add a site.</Typography></Paper>
          }
          <div>
          {(!(this.state.modesLoading || this.state.eventsLoading || this.state.sitesLoading || this.state.connectedLoading || this.state.timezonesLoading) && this.state.connectionType == "FM") &&

            <div style={{ width: '95%', margin: 'auto' }}>

              <FormControl 
                disabled={false}
                style={{ minWidth: '240px'}} 
                variant="outlined">
                <InputLabel>Site</InputLabel>
                    <Select
                    label="Site"
                    name="Site"
                    disabled={false}

                    onChange={this.handleDropDown}
                    value={this.state.siteSelected}
                    >
                      {this.state.siteData.map((element => {
                        return <MenuItem value={element[1]}>
                          <div style={{display: 'flex', flexDirection: 'row', alignItems: 'center'}}>
                                {/* <ListItemIcon> */}
                                {this.state.siteConnection[element[1]] == false &&
                                  <ErrorIcon style={{color: 'red', marginRight: '10px'}} />}
                                {/* </ListItemIcon> */}
                                <Typography>{element[0]}</Typography>
                                </div>
                              </MenuItem>
                              
                      }))}
                    </Select>
                    {this.state.siteConnection[this.state.siteSelected] == false &&
                         <FormHelperText>Site is disconnected</FormHelperText>}
              </FormControl>
        
            </div>}
            
            {((this.state.connectionType == "SC" || (this.state.siteConnection[this.state.siteSelected] && this.state.connectionType == "FM")) && !displaySchedulerNeedsConfig && !displaySpinCircle) && <>
          <Timeline timezone={this.state.timezone} title="Today" eventsToday={this.state.eventsToday}
                      modeData={this.state.modeData} eventsTEST={this.state.eventsTEST} eventsTomorrow={this.state.eventsTomorrow} selectEvent={this.selectEvent} selectedEvent={this.state.selectedEvent} selectedEventDayIsToday={this.state.selectedEventDayIsToday} time={this.state.time}/>
          <Timeline timezone={this.state.timezone} title="Tomorrow" eventsToday={this.state.eventsToday}
                      modeData={this.state.modeData} eventsTEST={this.state.eventsTEST} eventsTomorrow={this.state.eventsTomorrow} selectEvent={this.selectEvent} selectedEvent={this.state.selectedEvent} selectedEventDayIsToday={this.state.selectedEventDayIsToday} time={this.state.time}/>
          <div style={{ height: '16px' }}/>
          <Paper style={{  width: '95%', height: '450px', margin: 'auto',   }} variant="outlined">
              <div style={{ borderColor: 'transparent', backgroundColor: 'transparent', float: 'left', width: '40%', height: '100%', position: 'relative', display: 'flex', flexDirection: 'column', alignItems: 'center' }}>
                  <EventList
                      // timezone={this.state.timezones[this.state.siteSelected]}
                      eventsToday={this.state.eventsToday}
                      eventsTomorrow={this.state.eventsTomorrow}
                      selectedEvent={this.state.selectedEvent}
                      selectedEventDayIsToday={this.state.selectedEventDayIsToday}
                      isToday={this.state.isTodaySelected}
                      modeData={this.state.modeData}
                      time={this.state.time}
                      selectEvent={this.selectEvent}
                      changeToToday={() => this.changeToToday}
                      changeToTomorrow={() => this.changeToTomorrow}
                      addEventButtonIsSelected={this.state.addEventButtonSelected}
                      addEventButtonSelected={() => this.addEventButtonSelected}
                      timezone={this.state.timezone}/>
              </div>
              <div style={{ borderColor: 'transparent', backgroundColor: 'transparent', width: '60%', height: '100%', position: 'relative', display: 'flex', flexDirection: 'column', alignItems: 'center' }}>
                  <FormList 
                  timezone={this.state.timezone}
                  modeData={this.state.modeData}
                  isTodaySelected={this.state.isTodaySelected}
                  selectedEvent={this.state.selectedEvent}
                  selectedEventDayIsToday={this.state.selectedEventDayIsToday}
                  addEventButtonSelected={this.state.addEventButtonSelected}
                  submitEvent={this.submitEvent}
                  cancelAddEvent={this.cancelAddEvent}
                  deleteEvent={this.deleteEvent}
                  isOverlappingStartTime={this.isOverlappingStartTime}
                  isOverlappingDuration={this.isOverlappingDuration}
                  time={this.state.time}
                  />
              </div>
          </Paper></>}


            </div>
            </>
            
        )
    }
} 

export default withStyles(LoadingHOC(Scheduler), STYLES_SCHEDULER);