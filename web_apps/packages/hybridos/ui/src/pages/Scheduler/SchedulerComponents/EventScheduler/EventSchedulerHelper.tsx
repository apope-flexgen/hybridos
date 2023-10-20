/* eslint-disable max-lines */
/* eslint-disable no-param-reassign */
import { Timezones, Views } from '@flexgen/storybook';
import dayjs, { Dayjs, ManipulateType, OpUnitType } from 'dayjs';
import { ApiMode, RepeatForAPI, SchedulerEvent } from 'shared/types/dtos/scheduler.dto';
import {
  DaysSelected,
  EditEventState,
  SchedulerUrls,
  VariableValues,
} from 'src/pages/Scheduler/SchedulerTypes';
import { v4 as uuid } from 'uuid';
import timezone from 'dayjs/plugin/timezone';
import utc from 'dayjs/plugin/utc';

dayjs.extend(utc);
dayjs.extend(timezone);

export const schedulerURLS = {
  getEvents: '/scheduler/events' as SchedulerUrls,
  addEvent: '/scheduler/events' as SchedulerUrls,
  deleteEvent: '/scheduler/events' as SchedulerUrls,
  getModes: '/scheduler/modes' as SchedulerUrls,
  addMode: '/scheduler/modes' as SchedulerUrls,
  deleteMode: '/scheduler/modes' as SchedulerUrls,
  getConfiguration: '/scheduler/configuration' as SchedulerUrls,
  postConfiguration: '/scheduler/configuration' as SchedulerUrls,
  getTimezones: '/scheduler/timezones' as SchedulerUrls,
  getConnected: '/scheduler/connected' as SchedulerUrls,
};

export const views = {
  week: 'week' as Views,
  month: 'month' as Views,
  twoWeeks: '2weeks' as Views,
  day: 'day' as Views,
  minute: 'minutes',
};

export const getStartAndEndTimeFrame = (
  value: Dayjs | null, 
  view: Views,
  timezone: Timezones
) => {
  dayjs.tz.setDefault(timezone)
  let startTime: Dayjs = dayjs.tz(value).subtract(48, 'h');
  let endTime: Dayjs = dayjs.tz(value).add(48, 'h');

  if (view === (views.month as Views) || view === (views.twoWeeks as Views)) {
    startTime = dayjs.tz(value)
      .startOf(views.week as OpUnitType)
      .subtract(48, 'h');
    const endOfMonth = dayjs.tz(value).endOf(views.month as OpUnitType);
    endTime = dayjs.tz(endOfMonth)
      .add(1, views.day as ManipulateType)
      .endOf(views.month as OpUnitType);
  } else if (view === (views.week as Views)) {
    startTime = dayjs.tz(value)
      .startOf(views.week as OpUnitType)
      .subtract(48, 'h');
    endTime = dayjs.tz(value).endOf(views.week as OpUnitType);
  }

  return { startTime, endTime };
};

export const handleVariableValues = (
  variableValues: VariableValues[],
  modeId: string,
  modes: ApiMode,
) => {
  const typedVariableValues: VariableValues[] = [];
  const mode = modes[modeId];
  mode.variables.map((modeVariable) => {
    variableValues.map((variableValue) => {
      if (variableValue.name === modeVariable.id) {
        if (modeVariable.type === 'Bool') {
          if (variableValue.value === 'true') typedVariableValues.push({ name: variableValue.name, value: true });
          else typedVariableValues.push({ name: variableValue.name, value: false });
        } else if (modeVariable.type === 'Float') {
          typedVariableValues.push({
            name: variableValue.name,
            value: parseFloat(variableValue.value.toString()),
          });
        } else if (modeVariable.type === 'Int') {
          typedVariableValues.push({
            name: variableValue.name,
            value: parseInt(variableValue.value.toString(), 10),
          });
        } else typedVariableValues.push({ name: variableValue.name, value: variableValue.value });
      }
      return variableValue;
    });
    return modeVariable;
  });
  return typedVariableValues;
};

export const createRepeatObject = (state: EditEventState, exceptions?: string[], id?: string) => {
  const cycle: 'day' | 'week' = state.repeatEveryIncrement.Weeks ? 'week' : 'day';
  const frequency: number = parseInt(state.repeatEveryValue, 10);

  let repeat: RepeatForAPI = {
    cycle,
    frequency,
    exceptions,
    id: id || undefined,
  };
  const bitArray: number[] = [];
  // eslint-disable-next-line @typescript-eslint/no-unused-expressions
  Object.keys(state.daysSelected).forEach((key) => {
    // eslint-disable-next-line @typescript-eslint/no-unused-expressions
    state.daysSelected[key as keyof DaysSelected] ? bitArray.push(1) : bitArray.push(0);
  });
  const dayMask = parseInt(bitArray.join(''), 2);
  repeat = { ...repeat, day_mask: dayMask };

  if (!state.endsOption.Never) {
    const endTime: string = state.endsOption.On && state.endOnDate !== null
      ? state.endOnDate.format()
      : '2000-01-01T00:00:00Z';
    const endCount: number = state.endsOption.After ? parseInt(state.endAfterOccurrences, 10) : 0;
    repeat = { ...repeat, end_time: endTime, end_count: endCount };
  } else {
    repeat = { ...repeat, end_count: -1 };
  }

  return repeat;
};

const padStart = (num: number, size: number) => `000000${num}`.substr(`000000${num}`.length - size);

const checkIfInExceptions = (exceptionsArray: string[], startTime: string) => {
  let returnVar = false;
  // TODO: fix lint
  // eslint-disable-next-line array-callback-return
  exceptionsArray.map((exception) => {
    if (dayjs(startTime).isSame(dayjs(exception), 'day')) returnVar = true;
  });
  return returnVar;
};

const checkIfBeforeEndDate = (currentStartTime: dayjs.Dayjs, endTime: string) => {
  if (endTime === '2000-01-01T00:00:00Z') return false;
  const endCheck = dayjs(endTime);
  return currentStartTime.isBefore(endCheck) || currentStartTime.isSame(endCheck, 'date');
};

const checkIfBeforeEndCount = (numOccurences: number, endCount: number) => {
  if (endCount === -1) return true;
  if (endCount === 0) return false;
  return numOccurences < endCount;
};

const createRecurringEvents = (
  event: SchedulerEvent,
  recurringStartTime: dayjs.Dayjs,
  endTimeAsDayJs: dayjs.Dayjs,
  numOccurences: number,
  tempEventsData: SchedulerEvent[],
  startTimeAsDayJs: dayjs.Dayjs,
  startTimeFromEvent: dayjs.Dayjs,
) => {
  if (event.repeat && event.repeat.frequency) {
    if (event.repeat.cycle && event.repeat.cycle === 'day') {
      recurringStartTime = recurringStartTime.add(event.repeat.frequency, 'days');
      const recurringEndTime = recurringStartTime.add(event.duration, 'minutes');
      if (
        (recurringStartTime.isAfter(startTimeAsDayJs)
          || recurringStartTime.isSame(startTimeAsDayJs))
        && (recurringEndTime.isBefore(endTimeAsDayJs) || recurringEndTime.isSame(endTimeAsDayJs))
        && ((event.repeat.end_time
          && checkIfBeforeEndDate(recurringStartTime, event.repeat.end_time))
          || (event.repeat.end_count
            && checkIfBeforeEndCount(numOccurences, event.repeat.end_count)))
        && (event.repeat.exceptions === undefined
          || !checkIfInExceptions(event.repeat.exceptions, recurringStartTime.format()))
      ) {
        const recurringEventInframe: SchedulerEvent = {
          id: uuid(),
          duration: event.duration,
          mode: event.mode,
          start_time: recurringStartTime.format(),
          variables: event.variables,
          repeat: event.repeat,
        };
        tempEventsData.push(recurringEventInframe);
      }
      numOccurences += 1;
    } else if (event.repeat.cycle === 'week') {
      const binary = `${Number(event.repeat.day_mask).toString(2)}`;
      const paddedBinary = padStart(parseInt(binary, 10), 7);

      const numArray = paddedBinary.split('');

      // eslint-disable-next-line @typescript-eslint/no-loop-func
      numArray.map((day, index) => {
        if (day === '1') {
          recurringStartTime = recurringStartTime.day(index);
          const recurringEndTime = recurringStartTime.add(event.duration, 'minutes');
          if (
            (recurringStartTime.isAfter(startTimeAsDayJs)
              || recurringStartTime.isSame(startTimeAsDayJs))
            && recurringStartTime.isAfter(startTimeFromEvent)
            && (recurringEndTime.isBefore(endTimeAsDayJs)
              || recurringEndTime.isSame(endTimeAsDayJs))
            && (recurringEndTime.isBefore(endTimeAsDayJs)
              || recurringEndTime.isSame(endTimeAsDayJs))
            && ((event.repeat?.end_time
              && checkIfBeforeEndDate(recurringStartTime, event.repeat.end_time))
              || (event.repeat?.end_count
                && checkIfBeforeEndCount(numOccurences, event.repeat.end_count)))
            && (event.repeat?.exceptions === undefined
              || !checkIfInExceptions(event.repeat.exceptions, recurringStartTime.format()))
          ) {
            const recurringEventInframe: SchedulerEvent = {
              id: uuid(),
              duration: event.duration,
              mode: event.mode,
              start_time: recurringStartTime.format(),
              variables: event.variables,
              repeat: event.repeat,
            };
            tempEventsData.push(recurringEventInframe);
          }
          if (recurringStartTime.isAfter(startTimeFromEvent)) numOccurences += 1;
        }
        return recurringStartTime;
      });
      const startOfWeek = recurringStartTime
        .startOf('w')
        .add(event.repeat.frequency, 'weeks')
        .format('MM-DD-YYYY');
      const startTime = recurringStartTime.format('HH:mm:ss');
      recurringStartTime = dayjs(`${startOfWeek} ${startTime}`);
    }
    if (recurringStartTime.isBefore(endTimeAsDayJs)) {
      createRecurringEvents(
        event,
        recurringStartTime,
        endTimeAsDayJs,
        numOccurences,
        tempEventsData,
        startTimeAsDayJs,
        startTimeFromEvent,
      );
    }
  }
};

export const getAllEventsInTimeFrame = (
  startTime: Dayjs,
  endTime: Dayjs,
  schedulerEventsData: SchedulerEvent[],
): SchedulerEvent[] => {
  const filteredEventsData: SchedulerEvent[] = [];

  schedulerEventsData.map((event) => {
    const startTimeFromEvent = dayjs(event.start_time);
    const endTimeFromEvent = startTimeFromEvent.add(event.duration, 'minutes');
    if (
      (startTimeFromEvent.isAfter(startTime) || startTimeFromEvent.isSame(startTime))
      && (endTimeFromEvent.isBefore(endTime) || endTimeFromEvent.isSame(endTime))
      && (event.repeat?.exceptions === undefined
        || !checkIfInExceptions(event.repeat?.exceptions, startTimeFromEvent.format()))
    ) {
      filteredEventsData.push(event);
    }
    if (!(event.repeat?.end_count === 1)) {
      const recurringStartTime = startTimeFromEvent;
      const numOccurences = 1;

      createRecurringEvents(
        event,
        recurringStartTime,
        endTime,
        numOccurences,
        filteredEventsData,
        startTime,
        startTimeFromEvent,
      );
    }
    return event;
  });
  return filteredEventsData;
};
