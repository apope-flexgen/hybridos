// TODO: fix lint
/* eslint-disable max-lines */
import { ThemeType, Timezones } from '@flexgen/storybook';
import { createTheme, Theme } from '@mui/material/styles';
import dayjs from 'dayjs';
import { SchedulerEvent, schedulerEventForAPI } from 'shared/types/dtos/scheduler.dto';
import { getNextRepeat } from './SchedulerComponents/EditEventModal/EditEventModal-helpers';
import {
  VariableValues,
  SchedulerTabs,
  Tab,
  ApplyChangesTo,
  ButtonSizes,
  ButtonColors,
  DayCircleIndex,
  DayCircleLabels,
  EditEventState,
} from './SchedulerTypes';

export const getEventWithException = (
  event: SchedulerEvent,
  startTime: string,
): schedulerEventForAPI => {
  if (event.repeat) {
    return {
      duration: event.duration,
      mode: event.mode,
      start_time:
        event.start_time !== event.repeat.start ? event.repeat.start : getNextRepeat(event),
      variables: event.variables,
      repeat: {
        ...event.repeat,
        id: undefined,
        exceptions: event.repeat.exceptions ? [...event.repeat.exceptions, startTime] : [startTime],
      },
    };
  }
  return event;
};

export const checkIfStartBeforeEnd = (state: EditEventState): boolean => {
  if (
    state.date === null
    || state.startMinutes === ''
    || state.startHours === ''
    || state.endMinutes === ''
    || state.endHours === ''
    || state.endDate === null
  ) return false;
  const startTimeOfNewEvent = dayjs(`${state.date?.format('YYYY-MM-DD')} ${state.startTime}`);
  const endTimeOfNewEvent = dayjs(`${state.endDate?.format('YYYY-MM-DD')} ${state.endTime}`);
  return (
    startTimeOfNewEvent.isAfter(endTimeOfNewEvent) || startTimeOfNewEvent.isSame(endTimeOfNewEvent)
  );
};

export const checkIfStartOrEndInPast = (state: EditEventState, timezone: Timezones): boolean => {
  if (
    state.date === null
    || state.endDate === null
    || state.startMinutes === ''
    || state.startHours === ''
    || state.endMinutes === ''
    || state.endHours === ''
  ) return false;
  const startTimeOfNewEvent = dayjs.tz(
    `${state.date?.format('YYYY-MM-DD')} ${state.startTime}`,
    timezone,
  );
  const endTimeOfNewEvent = dayjs.tz(
    `${state.endDate?.format('YYYY-MM-DD')} ${state.endTime}`,
    timezone,
  );
  const now = dayjs().tz(timezone).subtract(1, 'minute');
  return startTimeOfNewEvent.isBefore(now) || endTimeOfNewEvent.isBefore(now);
};

export const checkIfEndInPast = (state: EditEventState, timezone: Timezones): boolean => {
  if (state.endDate === null || state.endMinutes === '' || state.endHours === '') return false;
  const endTimeOfNewEvent = dayjs.tz(
    `${state.endDate?.format('YYYY-MM-DD')} ${state.endTime}`,
    timezone,
  );
  const now = dayjs().tz(timezone).subtract(1, 'minute');
  return endTimeOfNewEvent.isBefore(now);
};

export function createMuiTheme(theme: ThemeType): Theme {
  return createTheme({
    typography: {
      h5: {
        fontFamily: theme.fgb.editModal.fonts.header.fontFamily,
        fontSize: `${theme.fgb.editModal.fonts.header.fontSize}px`,
        fontWeight: theme.fgb.editModal.fonts.header.fontWeight,
        color: theme.fgc.modal.color.text,
      },
      body1: {
        fontFamily: theme.fgb.editModal.fonts.label.fontFamily,
        fontSize: `${theme.fgb.editModal.fonts.label.fontSize}px`,
        fontWeight: theme.fgb.editModal.fonts.label.fontWeight,
        color: theme.fgc.modal.color.text,
      },
    },
  });
}

export const daysList: { name: DayCircleIndex; label: DayCircleLabels }[] = [
  { name: 'sun', label: 'S' },
  { name: 'mon', label: 'M' },
  { name: 'tues', label: 'T' },
  { name: 'wed', label: 'W' },
  { name: 'thurs', label: 'T' },
  { name: 'fri', label: 'F' },
  { name: 'sat', label: 'S' },
];

export const initialRepeatEveryOptions = {
  Days: true,
  Weeks: false,
};

export const initialEndsOnOptions = {
  Never: true,
  On: false,
  After: false,
};

export const initialApplyChangesTo: ApplyChangesTo = {
  thisEventOnly: true,
  allInSeries: false,
};

export const initialDaysSelected = {
  sun: false,
  mon: false,
  tues: false,
  wed: false,
  thurs: false,
  fri: false,
  sat: false,
};

export const schedulerLabels = {
  card: {
    label: 'EVENT SCHEDULE',
    icon: 'CalendarToday',
  },
  schedulerTab: {
    value: 'scheduler' as SchedulerTabs,
  },
  schedulerConfigTab: {
    value: 'scheduler_config' as SchedulerTabs,
  },
  modeManagerTab: {
    value: 'mode_manager' as SchedulerTabs,
  },
  unconfiguredContainer: {
    adminLabel:
      'Event Scheduler is not configured. As an admin, you may set up the configuration now.',
    userLabel: 'Event Scheduler is not configured. Contact an admin to set up the configuration.',
    adminButton: {
      label: 'Begin Configuration',
      color: 'primary' as ButtonColors,
      size: 'large' as ButtonSizes,
    },
  },
  fleetManager: {
    siteSelector: 'Select Schedule',
    siteDisconnected: 'Selected Site is Disconnected',
    noSiteSelected: 'Select Site to View Event Scheduler',
  },
};

export const getVariables = (variableValues: VariableValues[]) => {
  let tempVariables = {};
  variableValues.forEach((variable) => {
    if (variable.batch_value) {
      tempVariables = {
        ...tempVariables,
        [variable.name]: {
          value: variable.value,
          batch_value: variable.batch_value,
        },
      };
    } else {
      tempVariables = {
        ...tempVariables,
        [variable.name]: variable.value,
      };
    }
  });
  return tempVariables;
};

export const defaultMode = 'default';

export const schedulerTabs: Tab[] = [
  { label: 'Event Schedule', value: 'scheduler' },
  { label: 'Mode Manager', value: 'mode_manager', icon: 'AdminPanel' },
  { label: 'Scheduler Configuration', value: 'scheduler_config', icon: 'AdminPanel' },
];

export const getColorList = (theme: ThemeType) => ({
  orange: theme.fgc.scheduler.customColors.orange,
  lightGreen: theme.fgc.scheduler.customColors.lightGreen,
  teal: theme.fgc.scheduler.customColors.teal,
  lightBlue: theme.fgc.scheduler.customColors.lightBlue,
  indigo: theme.fgc.scheduler.customColors.indigo,
  deepPurple: theme.fgc.scheduler.customColors.deepPurple,
  red: theme.fgc.scheduler.customColors.red,
  pink: theme.fgc.scheduler.customColors.pink,
  deepOrange: theme.fgc.scheduler.customColors.deepOrange,
  gray: theme.fgc.scheduler.customColors.gray,
});

export const initialEditState: EditEventState = {
  startTime: '',
  endTime: '',
  endDate: null,
  endHours: '',
  endMinutes: '',
  startHours: '',
  startMinutes: '',
  date: null,
  mode: '',
  // FIXME: is this right
  modeId: '',
  variableValues: [],
  repeatEveryValue: '1',
  repeatEveryIncrement: initialRepeatEveryOptions,
  daysSelected: initialDaysSelected,
  endOnDate: null,
  endsOption: initialEndsOnOptions,
  endAfterOccurrences: '',
  discardClicked: false,
};
