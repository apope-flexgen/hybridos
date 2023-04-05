/* eslint-disable max-lines */
// TODO: fix lint
/* eslint-disable max-len */
import {
  CardRow, Views, TimeZones, Select, Typography, ThemeType, Box
} from '@flexgen/storybook';
import dayjs, { Dayjs } from 'dayjs';
import {
  createContext, useCallback, useEffect, useState, useContext,
} from 'react';
import {
  Repeat, SchedulerEvent, schedulerEventForAPI,
} from 'shared/types/dtos/scheduler.dto';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import { useSchedulerContext } from 'src/pages/Scheduler/Scheduler';
import AddEvent from 'src/pages/Scheduler/SchedulerComponents/AddEvent/AddEvent';
import CalendarGroup from 'src/pages/Scheduler/SchedulerComponents/CalendarGroup/CalendarGroup';
import {
  getAllEventsInTimeFrame, getStartAndEndTimeFrame, schedulerURLS, views,
} from 'src/pages/Scheduler/SchedulerComponents/EventScheduler/EventSchedulerHelper';
import { noSelectedItemBoxSx, schedulerLabels } from 'src/pages/Scheduler/SchedulerHelpers';
import { FleetManagerSite } from 'src/pages/Scheduler/SchedulerTypes';
import { useTheme } from 'styled-components';
import { NotifContextType, NotifContext } from 'src/contexts/NotifContext';

interface EventSchedulerProps {
  setIsLoading: any
  schedulerType: 'SC' | 'FM' | null
}

interface EventContextValue {
  eventsForUi: SchedulerEvent[]
  addEvent: (event: schedulerEventForAPI) => void
  updateEvent: (eventId: string, updatedEvent: schedulerEventForAPI) => void
  addException: (eventId: string, exception: string) => void
  view: Views
  setView: (newView: Views) => void
  value: dayjs.Dayjs | null
  setValue: (newValue: dayjs.Dayjs | null) => void
  timezone: TimeZones[]
  siteId: string
  fmSites: FleetManagerSite[],
  setSiteId: (newValue: string) => void
  getEventsWithinTimeFrame: () => void
}

const EventSchedulerContext = createContext<EventContextValue>({
  eventsForUi: [],
  addEvent: () => {},
  updateEvent: () => {},
  addException: () => {},
  view: views.week,
  setView: () => {},
  value: null,
  setValue: () => {},
  timezone: [],
  siteId: '',
  fmSites: [],
  setSiteId: () => {},
  getEventsWithinTimeFrame: () => {},
});

export function useEventSchedulerContext() {
  return useContext(EventSchedulerContext);
}

const EventScheduler = ({ setIsLoading, schedulerType }: EventSchedulerProps) => {
  const [view, setView] = useState<Views>(views.week);
  const [value, setValue] = useState<Dayjs | null>(dayjs());
  const [eventsForUI, setEventsForUI] = useState<SchedulerEvent[]>([]);
  const [selectedSiteName, setSelectedSiteName] = useState<string>('');
  const notifCtx = useContext<NotifContextType | null>(NotifContext);

  const {
    siteId, timezone, events, modes, fmSites, setSiteId,
  } = useSchedulerContext();

  const axiosInstance = useAxiosWebUIInstance();
  const theme = useTheme() as ThemeType;

  // get all events that will occur within the time frame displayed
  const getEventsWithinTimeFrame = useCallback(() => {
    setIsLoading(true);
    const { startTime, endTime } = getStartAndEndTimeFrame(value, view);

    const eventsFromAPI = (events === null || events[siteId] === undefined) ? [] : events[siteId];

    const eventsWithRepeatStartTimes: SchedulerEvent[] = [];

    eventsFromAPI.map((event) => {
      const newRepeat: Repeat = { ...event.repeat, start: event.start_time };
      const newEvent = { ...event, repeat: newRepeat };
      eventsWithRepeatStartTimes.push(newEvent);
      return event;
    });

    const newEventsForUI = getAllEventsInTimeFrame(
      startTime,
      endTime,
      eventsWithRepeatStartTimes,
    );

    setEventsForUI(newEventsForUI);
    setIsLoading(false);
  // TODO: Fix eslint-ignore
  // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [siteId, value, view, axiosInstance, events, modes, setIsLoading]);

  const addEvent = (newEvent: schedulerEventForAPI) => {
    axiosInstance.post(`${schedulerURLS.addEvent}/${siteId}`, newEvent)
    .then((res)=>{
      if (res.data.body.error) notifCtx?.notif('error', res.data.body.error); 
    });
  };

  const updateEvent = (eventId: string, eventBody: schedulerEventForAPI) => {
    axiosInstance.post(`${schedulerURLS.addEvent}/${siteId}/${eventId}`, eventBody)
    .then((res)=>{
      if (res.data.body.error) notifCtx?.notif('error', res.data.body.error); 
    });
  };

  const addException = (eventId: string, exception: string) => {
    axiosInstance.post(`${schedulerURLS.addEvent}/${siteId}/${eventId}/exceptions`, { data: exception })
    .then((res)=>{
      if (res.data.body.error) notifCtx?.notif('error', res.data.body.error); 
    });
  };

  // eslint-disable-next-line react/jsx-no-constructed-context-values
  const contextValue = {
    eventsForUi: eventsForUI,
    addEvent,
    updateEvent,
    addException,
    view,
    setView,
    value,
    setValue,
    timezone,
    siteId,
    fmSites,
    setSiteId,
    getEventsWithinTimeFrame,
  };

  // whenever the user moves to a different date or view,
  // get the events that occur within that timeframe
  useEffect(() => {
    getEventsWithinTimeFrame();
  // TODO: fix eslint-ignore
  // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [view, value, events]);

  const siteNames: string[] = [];
  fmSites.map((obj: FleetManagerSite) => siteNames.push(obj.name));

  const changeSite = (siteName: string) => {
    setSelectedSiteName(siteName);
    const site: any = fmSites.find((obj: FleetManagerSite) => obj.name === siteName);
    if (site) setSiteId(site.id);
  };

  useEffect(() => {
    if (siteId) {
      const site: any = fmSites.find((obj: FleetManagerSite) => obj.id === siteId);
      if (site) setSelectedSiteName(site.name);
    }
    // TODO: Fix eslint-ignore
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  return (
    <EventSchedulerContext.Provider value={contextValue}>
      {schedulerType === 'FM'
          && (
            <div style={{
              padding: '16px', paddingLeft: '32px', display: 'flex', flexDirection: 'column', gap: '16px', maxWidth: '350px',
            }}
            >
              <Typography variant="headingS" text="Site Selector" />
              <Select
                label={schedulerLabels.fleetManager.siteSelector}
                menuItems={siteNames}
                value={selectedSiteName}
                onChange={(e) => changeSite(e.target.value)}
              />
            </div>
          )}
      <CardRow alignItems="flex-start" direction="row">
        {siteId
          ? (
            <>
              <AddEvent />
              <CalendarGroup />
            </>
          )
          : (
            <Box sx={noSelectedItemBoxSx(theme)}>
              <Typography text={schedulerLabels.fleetManager.noSiteSelected} variant="headingM" />
            </Box>
          )}
      </CardRow>
    </EventSchedulerContext.Provider>
  );
};

export default EventScheduler;
