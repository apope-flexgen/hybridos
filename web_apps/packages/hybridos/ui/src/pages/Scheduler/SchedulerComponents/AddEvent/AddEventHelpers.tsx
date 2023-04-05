/* eslint-disable max-lines */
import { SelectChangeEvent } from '@mui/material';
import dayjs, { Dayjs } from 'dayjs';
import {
  ApiMode, schedulerEventForAPI, RepeatForAPI, SchedulerEvent,
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
  title: { label: 'CREATE NEW EVENT' },
  eventType: { label: 'Mode' },
  startDate: { label: 'Start Date' },
  endDate: { label: 'End Date' },
  repeatEveryRadios: { dayLabel: 'Days', weekLabel: 'Weeks' },
  repeatEvery: {
    placeholder: 1,
    validationRegEx: 'positiveIntegers' as keyof typeof numberValidationEnum,
  },
  endOptionsRadios: { neverLabel: 'Never', onLabel: 'On', afterLabel: 'After' },
  clearButton: { label: 'CLEAR', size: 'medium', variant: 'text' },
  addEventButton: { label: 'ADD EVENT', size: 'medium', startIcon: 'Add' },
  occurencesNumericInput: {
    label: 'Events',
    validationRegEx: 'positiveIntegers' as keyof typeof numberValidationEnum,
  },
  startAfterEndError: { label: 'End time of event must be after start time' },
  durationOver24Error: { label: 'Duration of event must be fewer than 24 hours' },
};

export const dayNames = ['sun', 'mon', 'tues', 'wed', 'thurs', 'fri', 'sat'];

export const mapModesToVariables = (
  e: SelectChangeEvent<string>,
  modesFromAPI: ApiMode,
  setVariables: React.Dispatch<React.SetStateAction<EventVariables[]>>,
  dispatch: React.Dispatch<any>,
) => {
  const tempVariableValues: VariableValues[] = [];
  Object.keys(modesFromAPI).map((modeId: string) => {
    if (modesFromAPI[modeId].name === e.target.value) {
      const { variables } = modesFromAPI[modeId];
      setVariables(variables);
      variables.map((variable) => {
        const { id, value } = variable;
        tempVariableValues.push({
          name: id,
          value: value.toString(),
        });
        return null;
      });
      dispatch({ type: 'setVariableValues', payload: tempVariableValues });
    }
    return null;
  });
};

export const isOverlappingStartTime = (
  date: Dayjs | null,
  startTime: string,
  endTime: string,
  endDate: Dayjs | null,
  events: SchedulerEvent[],
) => {
  let overlapping = false;
  if (startTime !== '' && endTime !== '') {
    const startTimeOfNewEvent = dayjs(`${date?.format('YYYY-MM-DD')} ${startTime}`);
    const endTimeOfNewEvent = dayjs(`${endDate?.format('YYYY-MM-DD')} ${endTime}`);
    events.map((existingEvent) => {
      const startTimeOfExistingEvent = dayjs(existingEvent.start_time);
      const endTimeOfExistingEvent = dayjs(existingEvent.start_time).add(existingEvent.duration, 'm');
      if (
        (startTimeOfNewEvent.isBefore(endTimeOfExistingEvent)
        || startTimeOfNewEvent.isSame(endTimeOfExistingEvent))
          && (endTimeOfNewEvent.isAfter(startTimeOfExistingEvent)
        || endTimeOfNewEvent.isSame(startTimeOfExistingEvent))
      ) overlapping = true;
      return overlapping;
    });
  }
  return overlapping;
};

export const isDurationOver24Hours = (
  state: EditEventState,
) => {
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
