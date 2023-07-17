import { IconList } from '@flexgen/storybook';
import { numberValidationEnum } from './SchedulerComponents/AddEvent/AddEventHelpers';
import {
  Tab, MoveDirections, ButtonColors, ButtonVariants,
} from './SchedulerTypes';

export const addEventLabels = {
  title: { label: 'Create New Event' },
  eventType: { label: 'Mode' },
  date: { label: 'Date' },
  repeatEveryRadios: { dayLabel: 'Days', weekLabel: 'Weeks' },
  repeatEvery: {
    placeholder: 1,
    validationRegEx: 'positiveIntegers' as keyof typeof numberValidationEnum,
  },
  endOptionsRadios: { neverLabel: 'Never', onLabel: 'On', afterLabel: 'After' },
  clearButton: { label: 'CLEAR', size: 'medium', variant: 'text' },
  addEventButton: { label: 'ADD', size: 'medium', startIcon: 'Add' },
  occurencesNumericInput: {
    label: 'Events',
    validationRegEx: 'positiveIntegers' as keyof typeof numberValidationEnum,
  },
};

export const calendarGroupLabels = {
  nextButton: {
    icon: 'ChevronRight' as IconList,
    move: 'next' as MoveDirections,
  },
  prevButton: {
    icon: 'ChevronLeft' as IconList,
    move: 'prev' as MoveDirections,
  },
  todayButton: { label: 'TODAY', move: 'today' as MoveDirections },
};

export const editEventLabels = {
  eventType: { label: 'Mode' },
  title: { pastEventLabel: 'View Past Event', activeEventLabel: 'Edit Active Event', eventLabel: 'Edit Event' },
  deleteButton: {
    icon: 'Trash' as IconList,
    seriesIcon: 'DeleteSeries' as IconList,
    color: 'error' as ButtonColors,
    variant: 'text' as ButtonVariants,
    recurringLabel: 'DELETE SERIES',
    eventLabel: 'DELETE EVENT',
  },
  confirmCancelButton: {
    confirmLabel: 'Confirm Delete',
    cancelIcon: 'Close' as IconList,
    confirmIcon: 'Check' as IconList,
    cancelLabel: 'Undo',
    confirmColor: 'error' as ButtonColors,
    cancelColor: 'inherit' as ButtonColors,
  },
  discardButton: { label: 'DISCARD', variant: 'text' as ButtonVariants },
  saveButton: { label: 'SAVE' },
  closeButton: { icon: 'Close' as IconList },
  applyChangesTo: { text: 'Apply Changes to' },
};

export const tabs: Tab[] = [
  { label: 'Daily', value: 'day' },
  { label: 'Week', value: 'week' },
  { label: '2 Weeks', value: '2weeks' },
  { label: 'Month', value: 'month' },
];
