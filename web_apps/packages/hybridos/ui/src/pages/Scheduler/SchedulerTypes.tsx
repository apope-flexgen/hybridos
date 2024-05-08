import { IconList } from '@flexgen/storybook';
import { Dayjs } from 'dayjs';

export type FleetManagerSite = {
  id: string;
  name: string;
  fleetSchedule?: boolean;
};

export type MoveDirections = 'next' | 'prev' | 'today';

export type EditEventState = {
  startTime: string;
  endTime: string;
  startMinutes: string;
  endMinutes: string;
  startHours: string;
  endHours: string;
  endDate: Dayjs | null;
  date: Dayjs | null;
  mode: string;
  modeId: string;
  variableValues: VariableValues[];
  repeatEveryValue: string;
  repeatEveryIncrement: {
    Days: boolean;
    Weeks: boolean;
  };
  daysSelected: DaysSelected;
  endsOption: EndsOnOptions;
  endOnDate: Dayjs | null;
  endAfterOccurrences: string;
  discardClicked: boolean;
};

export type Mode = {
  id: string;
  name: string;
  color: string;
  borderColor: string;
  backgroundColor: string;
};
export interface EventVariables {
  id: string;
  name: string;
  type: string;
  unit: string;
  uri: string;
  value: number | boolean;
  batch_prefix?: string;
  batch_range?: string[];
  batch_value?: string[];
}
export interface VariableValues {
  name: string;
  value: string | boolean | number;
  batch_value?: string[];
}
export interface ApplyChangesTo {
  thisEventOnly: boolean;
  allInSeries: boolean;
}
export interface EndsOnOptions {
  Never: boolean;
  On: boolean;
  After: boolean;
}
export interface RepeatEveryOptions {
  Days: boolean;
}

export interface DaysSelected {
  sun: boolean;
  mon: boolean;
  tues: boolean;
  wed: boolean;
  thurs: boolean;
  fri: boolean;
  sat: boolean;
}

export interface Repeat {
  start: string;
  id: string;
  cycle: 'day' | 'week';
  frequency: number;
  dayMask?: number | undefined;
  end?: string | number | undefined;
  exceptions?: string[];
}

export type SchedulerUrls =
  | '/scheduler/events'
  | '/scheduler/deleteEvent'
  | '/scheduler/configuration'
  | '/scheduler/modes'
  | '/scheduler/timezones'
  | '/scheduler/connected';

export interface Tab {
  label: string;
  value: string;
  icon?: IconList;
}

export type ModeColors =
  | 'orange'
  | 'lightGreen'
  | 'teal'
  | 'lightBlue'
  | 'indigo'
  | 'deepPurple'
  | 'red'
  | 'pink'
  | 'deepOrange'
  | 'gray';

export type SchedulerTabs = 'scheduler' | 'scheduler_config' | 'mode_manager';
export type ButtonSizes = 'small' | 'medium' | 'large';
export type ButtonVariants = 'contained' | 'outlined' | 'text';
export type DayCircleLabels = 'S' | 'M' | 'T' | 'W' | 'TH' | 'F' | 'S';
export type DayCircleIndex = 'sun' | 'mon' | 'tues' | 'wed' | 'thurs' | 'fri' | 'sat' | 'sun';
