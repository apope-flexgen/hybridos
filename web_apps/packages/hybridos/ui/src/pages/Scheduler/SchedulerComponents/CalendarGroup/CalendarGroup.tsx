// TODO: fix lint
/* eslint-disable max-lines */
import {
  ThemeType, Views, createTemplate, Calendar,
} from '@flexgen/storybook';
import { Box } from '@mui/material';
import { EventObject } from '@toast-ui/calendar/*';
import dayjs, { ManipulateType } from 'dayjs';
import React, { useEffect, useState, useCallback } from 'react';
import { SchedulerEvent } from 'shared/types/dtos/scheduler.dto';
import { useSchedulerContext } from 'src/pages/Scheduler/Scheduler';
import EditEventModal from 'src/pages/Scheduler/SchedulerComponents/EditEventModal/EditEventModal';
import { useEventSchedulerContext } from 'src/pages/Scheduler/SchedulerComponents/EventScheduler/EventScheduler';
import { views } from 'src/pages/Scheduler/SchedulerComponents/EventScheduler/EventSchedulerHelper';
import { defaultMode, getColorList } from 'src/pages/Scheduler/SchedulerHelpers';
import { Mode, ModeColors } from 'src/pages/Scheduler/SchedulerTypes';
import { useTheme } from 'styled-components';
import { initialEventObject, recurringString } from './CalendarGroupHelpers';
import ViewDisplay from './ViewDisplay';

const CalendarGroup: React.FC = () => {
  const [displayEvents, setDisplayEvents] = useState<any[] | undefined>([]);
  const [displayModes, setDisplayModes] = useState<Mode[]>([]);
  const [editModalOpen, setEditModalOpen] = useState(false);
  const [editModalEvent, setEditModalEvent] = useState<SchedulerEvent>(initialEventObject);
  const [calendarHeight, setCalendarHeight] = useState<string>('50vh');
  const [windowSize, setWindowSize] = useState(window.innerHeight);

  useEffect(() => {
    const handleResize = () => setWindowSize(window.innerHeight);
    window.addEventListener('resize', handleResize);
    return () => window.removeEventListener('resize', handleResize);
  }, []);

  useEffect(() => {
    if (windowSize >= 900) return setCalendarHeight('61vh');
    if (windowSize >= 700) return setCalendarHeight('50vh');
    return setCalendarHeight('40vh');
  }, [windowSize]);

  const calendarRef = React.createRef();

  const { modes } = useSchedulerContext();
  const { eventsForUi, view, timezone } = useEventSchedulerContext();

  const theme = useTheme() as ThemeType;
  const colorList: { [color in ModeColors]: string } = getColorList(theme);

  // when an event is clicked on, open the modal, send in info regarding the event which was clicked
  const handleEventClick = useCallback(
    (e: EventObject) => {
      setEditModalEvent(eventsForUi[e.event.id]);
      setEditModalOpen(true);
    },
    [eventsForUi],
  );

  useEffect(() => {
    const newTemplate = createTemplate(theme, displayModes);
    // eslint-disable-next-line @typescript-eslint/ban-ts-comment
    // @ts-ignore
    const calendarInstance = calendarRef.current?.getInstance();
    calendarInstance?.setOptions({ template: newTemplate });
    calendarInstance?.render();
    // TODO: fix eslint-ignore
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [calendarRef, theme]);

  const getModeName = (event: SchedulerEvent) => {
    const id = Object.keys(modes).find((modeId: string) => modeId === event.mode);
    let name = '';
    if (id) name = modes[id].name;
    return name;
  };

  useEffect(() => {
    const tempDisplayEvents: any[] = [];
    // eslint-disable-next-line array-callback-return
    eventsForUi.map((event: any, index: any) => {
      const modeName = getModeName(event);
      const startTime = dayjs(event.start_time);
      const endTime = startTime.add(event.duration, views.minute as ManipulateType);
      tempDisplayEvents.push({
        id: index,
        calendarId: event.mode,
        title: modeName,
        category:
          view === (views.twoWeeks as Views) || view === (views.month as Views)
            ? ['task']
            : undefined,
        isReadOnly: true,
        start: startTime.toDate(),
        end: endTime.toDate(),
        recurrenceRule: event.repeat.end_count === 1 ? '' : recurringString,
        customStyle: {
          border:
            view === 'day' || view === 'week'
              ? `1px solid ${theme.fgc.datePicker.color.text}`
              : 'none',
          maxHeight:
            (view === 'day' || view === 'week') && event.duration <= 15
              ? 'calc(2.08333% - 13px)'
              : undefined,
        },
      });
    });
    setDisplayEvents(tempDisplayEvents);

    if (editModalOpen && editModalEvent.id !== undefined) {
      const currentOpenEvent = eventsForUi.find((event) => event.id === editModalEvent.id);
      if (currentOpenEvent === undefined) setEditModalOpen(false);
      else setEditModalEvent(currentOpenEvent);
    }
    // TODO: fix eslint ignore
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [eventsForUi, view, theme]);

  useEffect(() => {
    const tempDisplayModes: Mode[] = [];

    // eslint-disable-next-line array-callback-return
    Object.keys(modes).map((modeId: string) => {
      if (modes[modeId].name?.toLowerCase() !== defaultMode) {
        tempDisplayModes.push({
          id: modeId,
          name: modes[modeId].name,
          color: theme.fgc.datePicker.color.text,
          borderColor: modes[modeId].icon,
          backgroundColor: colorList[modes[modeId].color_code as ModeColors],
        });
      }
    });
    setDisplayModes(tempDisplayModes);
    // TODO: fix eslint-ignore
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [modes, theme]);

  // rule used for recurrence - if event end_count is 1, event does not repeat
  const recurrenceRule = !(editModalEvent.repeat?.end_count === 1);

  return (
    <Box sx={{ flexGrow: 1, paddingRight: '16px' }}>
      <ViewDisplay
        calendarRef={calendarRef}
        displayEvents={displayEvents}
        setDisplayEvents={setDisplayEvents}
      />
      <Box>
        <Calendar
          height={calendarHeight}
          calendarRef={calendarRef}
          events={displayEvents}
          modes={displayModes}
          onClickEvent={handleEventClick}
          timezone={timezone}
          view={view}
        />
      </Box>
      <EditEventModal
        event={editModalEvent}
        open={editModalOpen}
        recurring={recurrenceRule}
        setEditModalOpen={setEditModalOpen}
      />
    </Box>
  );
};

export default CalendarGroup;
