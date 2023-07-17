// TODO: fix lint
/* eslint-disable max-lines */
import {
  CardRow,
  Tab,
  Tabs,
  DatePicker,
  IconButton,
  MuiButton,
  ThemeType,
  Views,
} from '@flexgen/storybook';
import { Box, Typography } from '@mui/material';
import { Dayjs } from 'dayjs';
import React, { useState, useEffect, useCallback } from 'react';
import { useEventSchedulerContext } from 'src/pages/Scheduler/SchedulerComponents/EventScheduler/EventScheduler';
import { useTheme } from 'styled-components';
import {
  tabs,
  calendarGroupLabels,
  getTempEvents,
  getDisplayDate,
  MoveDirections,
  getCalendarDayJSDate,
} from './CalendarGroupHelpers';

interface ViewDisplayProps {
  displayEvents: any[] | undefined;
  setDisplayEvents: React.Dispatch<React.SetStateAction<any[] | undefined>>;
  calendarRef: React.RefObject<any>;
}

const ViewDisplay: React.FC<ViewDisplayProps> = ({
  displayEvents,
  setDisplayEvents,
  calendarRef,
}: ViewDisplayProps) => {
  const [dateDisplay, setDateDisplay] = useState('');
  const theme = useTheme() as ThemeType;

  const {
    view, setView, value, setValue,
  } = useEventSchedulerContext();

  const handleViewChange = useCallback(
    (_: any, newValue: unknown) => {
      setView(newValue as Views);
      const displayEventsTemp = getTempEvents(displayEvents, newValue);
      setDisplayEvents(displayEventsTemp);
    },
    [setView, displayEvents, setDisplayEvents],
  );

  const handleDateChange = useCallback(
    (newValue: Dayjs | null) => {
      setValue(newValue);
      const calendarInstance = calendarRef.current?.getInstance();
      const calDate = calendarInstance?.getDate();
      if (newValue) {
        switch (view) {
          case 'day':
            calDate.setFullYear(newValue.year(), newValue.month(), newValue.date());
            break;
          default:
            calendarInstance?.setDate(new Date(newValue.year(), newValue.month(), newValue.date()));
            break;
        }
      }
      calendarInstance?.render();
      if (calendarInstance) {
        const displayDate = getDisplayDate(calendarInstance, view);
        setDateDisplay(displayDate);
      }
    },
    [calendarRef, setValue, view],
  );

  const handleCalendarMove = (type: MoveDirections) => {
    const calendarInstance = calendarRef.current?.getInstance();
    if (type === 'prev') calendarInstance.prev();
    else if (type === 'next') calendarInstance.next();
    else calendarInstance.today();
    if (calendarInstance) {
      const dayJSDate = getCalendarDayJSDate(calendarInstance);
      setValue(dayJSDate);
      const displayDate = getDisplayDate(calendarInstance, view);
      setDateDisplay(displayDate);
    }
  };

  // set up initial date display and datepicker value
  useEffect(() => {
    handleDateChange(value);
  }, [handleDateChange, value]);

  return (
    <>
      <CardRow>
        <Tabs onChange={handleViewChange} value={view}>
          {tabs.map((tab) => (
            <Tab key={tab.value} label={tab.label} value={tab.value} />
          ))}
        </Tabs>
      </CardRow>
      <CardRow alignItems="center">
        <IconButton
          color="action"
          icon="ChevronLeft"
          onClick={() => handleCalendarMove(calendarGroupLabels.prevButton.move)}
        />
        <IconButton
          color="action"
          icon="ChevronRight"
          onClick={() => handleCalendarMove(calendarGroupLabels.nextButton.move)}
        />
        {/** FIXME: Use our typography when merged */}
        <Typography
          sx={{
            fontFamily: 'Saira',
            fontSize: 18,
            fontWeight: 600,
            color: theme.fgc.calendar.color.fonts.today,
            textTransform: 'uppercase',
          }}
        >
          {dateDisplay}
        </Typography>
        <Box
          sx={{
            marginLeft: 'auto',
            display: 'flex',
            gap: '16px',
            alignItems: 'center',
          }}
        >
          <MuiButton
            color="inherit"
            label={calendarGroupLabels.todayButton.label}
            onClick={() => handleCalendarMove(calendarGroupLabels.todayButton.move)}
            variant="outlined"
          />
          <DatePicker onChange={handleDateChange} renderInput="iconButton" value={value} />
        </Box>
      </CardRow>
    </>
  );
};

export default ViewDisplay;
