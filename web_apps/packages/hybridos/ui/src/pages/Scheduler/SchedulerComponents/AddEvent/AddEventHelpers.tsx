/* eslint-disable max-lines */
import { Timezones } from '@flexgen/storybook';
import { SelectChangeEvent } from '@mui/material';
import dayjs from 'dayjs';
import {
  ApiMode,
  schedulerEventForAPI,
  RepeatForAPI,
  SchedulerEvent,
} from 'shared/types/dtos/scheduler.dto';
import { getVariables } from 'src/pages/Scheduler/SchedulerHelpers';
import { EditEventState, EventVariables, VariableValues } from 'src/pages/Scheduler/SchedulerTypes';

export const numberValidationEnum = {
  positiveIntegers: /^[0-9]+$/,
  integers: /^-?[0-9]*(?![a-z])$/,
  negativeIntegers: /^-[0-9]*(?![a-z])$/,
  positiveFloats: /^[0-9]+(?![a-z]).?[0-9]*$/,
  floats: /^-?[0-9]*(?![a-z]).?[0-9]*$/,
  negativeFloats: /^-[0-9]*(?![a-z]).?[0-9]*$/,
};

export const addEventLabels = {
  title: { label: 'Create New Event' },
  eventType: { label: 'Mode' },
  startDate: { label: 'Start' },
  endDate: { label: 'End' },
  repeatEveryRadios: { dayLabel: 'Days', weekLabel: 'Weeks' },
  repeatEvery: {
    placeholder: 1,
    validationRegEx: 'positiveIntegers' as keyof typeof numberValidationEnum,
  },
  endOptionsRadios: { neverLabel: 'Never', onLabel: 'On', afterLabel: 'After' },
  clearButton: { label: 'Clear', size: 'medium', variant: 'text' },
  addEventButton: { label: 'Add Event', size: 'medium', startIcon: 'Add' },
  occurencesNumericInput: {
    label: 'Events',
    validationRegEx: 'positiveIntegers' as keyof typeof numberValidationEnum,
  },
  startAfterEndError: { label: 'End time of event must be after start time' },
  durationOver24Error: { label: 'Duration of event must be fewer than 24 hours' },
  cannotOverlapError: { label: 'New event cannot overlap with existing events' },
  startOrEndInPastError: { label: 'New event cannot start or end in the past' },
  endInPastError: { label: 'New event cannot have an end time in the past' },
};

export const dayNames = ['sun', 'mon', 'tues', 'wed', 'thurs', 'fri', 'sat'];

export const mapModesToVariables = (
  e: SelectChangeEvent<string>,
  modesFromAPI: ApiMode,
  setVariables: React.Dispatch<React.SetStateAction<EventVariables[]>>,
  dispatch: React.Dispatch<any>,
) => {
  const tempVariableValues: VariableValues[] = [];
  Object.keys(modesFromAPI).forEach((modeId: string) => {
    if (modesFromAPI[modeId].name === e.target.value) {
      const { variables } = modesFromAPI[modeId];
      setVariables(variables);
      variables.forEach((variable) => {
        // eslint-disable-next-line @typescript-eslint/naming-convention
        const { id, value, batch_value } = variable;
        tempVariableValues.push({
          name: id,
          value: value.toString(),
          batch_value,
        });
        return null;
      });
      dispatch({ type: 'setVariableValues', payload: tempVariableValues });
    }
    return null;
  });
};

export const determineBatchItemsFromRange = (prefix: string, uri: string, batchRange: string[]) => {
  let numericExtensions: { [key: string]: number } = {};
  const menuItems: string[] = [];
  const fullURI = `${prefix}${uri.split('#')[0]}`;
  batchRange.forEach((rangeItem) => {
    if (typeof rangeItem === 'string' && rangeItem.includes('..')) {
      const arrayOfRange = rangeItem.split('..');
      const startNumber = Number(arrayOfRange[0]);
      const endNumber = Number(arrayOfRange[1]);
      for (let i = startNumber; i < endNumber + 1; i += 1) {
        numericExtensions = {
          ...numericExtensions,
          [`${fullURI}${i}`]: i,
        };
        menuItems.push(`${fullURI}${i}`);
      }
    } else {
      numericExtensions = {
        ...numericExtensions,
        [`${fullURI}${rangeItem}`]: Number(rangeItem),
      };
      menuItems.push(`${fullURI}${rangeItem}`);
    }
  });
  return { numericExtensions, menuItems };
};

export const isOverlappingStartTime = (
  state: EditEventState,
  events: SchedulerEvent[],
  timezone: Timezones,
  editEvent?: SchedulerEvent,
) => {
  let overlapping = false;
  if (state.date && state.endDate && state.startTime !== '' && state.endTime !== '') {
    const startTimeOfNewEvent = dayjs.tz(
      `${state.date?.format('YYYY-MM-DD')} ${state.startTime}`,
      timezone,
    );
    const endTimeOfNewEvent = dayjs.tz(
      `${state.endDate?.format('YYYY-MM-DD')} ${state.endTime}`,
      timezone,
    );

    events.forEach((existingEvent) => {
      const startTimeOfExistingEvent = dayjs(existingEvent.start_time).tz(timezone);
      const endTimeOfExistingEvent = startTimeOfExistingEvent.add(existingEvent.duration, 'm');
      if (
        startTimeOfNewEvent.isBefore(endTimeOfExistingEvent)
        && endTimeOfNewEvent.isAfter(startTimeOfExistingEvent)
        && (editEvent === undefined || editEvent.id !== existingEvent.id)
      ) {
        overlapping = true;
      }
    });
  }
  return overlapping;
};

export const isDurationOver24Hours = (state: EditEventState) => {
  let over24 = false;
  if (state.startTime !== '' && state.endTime !== '') {
    const startTimeOfNewEvent = dayjs(`${state.date?.format('YYYY-MM-DD')} ${state.startTime}`);
    const endTimeOfNewEvent = dayjs(`${state.endDate?.format('YYYY-MM-DD')} ${state.endTime}`);
    const difference = endTimeOfNewEvent.diff(startTimeOfNewEvent, 'h');
    if (difference >= 24) over24 = true;
  }
  return over24;
};

export const onArrow = (
  direction: 'up' | 'down',
  repeatEveryValue: string,
  dispatch: React.Dispatch<any>,
) => {
  const newValue = direction === 'up' ? Number(repeatEveryValue) + 1 : Number(repeatEveryValue) - 1;
  const regEx = new RegExp(numberValidationEnum.positiveIntegers);
  if (regEx.test(`${newValue}`)) {
    dispatch({ type: 'setRepeatEveryValue', payload: newValue.toString() });
  }
};

export const onAfterOcrruencesArrow = (
  direction: 'up' | 'down',
  endAfterOccurrences: string,
  dispatch: React.Dispatch<any>,
) => {
  const newValue = direction === 'up' ? Number(endAfterOccurrences) + 1 : Number(endAfterOccurrences) - 1;
  const regEx = new RegExp(numberValidationEnum.positiveIntegers);
  if (regEx.test(`${newValue}`)) {
    dispatch({ type: 'setEndAfterOccurrences', payload: newValue.toString() });
  }
};

export const createNewEvent = (
  duration: number,
  eventType: string,
  start_time: string,
  variableValues: VariableValues[],
  repeat?: RepeatForAPI,
): schedulerEventForAPI => ({
  duration,
  mode: eventType,
  start_time,
  variables: getVariables(variableValues),
  repeat: repeat === undefined ? undefined : repeat,
});
