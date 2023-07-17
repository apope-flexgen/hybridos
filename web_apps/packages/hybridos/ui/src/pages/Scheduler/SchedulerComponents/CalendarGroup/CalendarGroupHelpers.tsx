import { IconList, Views } from '@flexgen/storybook';
import Calendar from '@toast-ui/calendar';
import dayjs, { Dayjs } from 'dayjs';
import { Tab } from 'src/pages/Scheduler/SchedulerTypes';

const handleWeekDisplay = (rangeStart: any, rangeEnd: any) => {
  const date = rangeStart.getDate();
  const dateEnd = rangeEnd.getDate();
  const month = rangeStart.getMonth();
  const monthEnd = rangeEnd.getMonth();
  const year = rangeStart.getFullYear();
  const yearEnd = rangeEnd.getFullYear();
  if (month !== monthEnd) {
    return `Week of ${dayjs().month(month).format('MMMM')} ${date}, ${year} - ${dayjs()
      .month(monthEnd)
      .format('MMMM')} ${dateEnd}, ${yearEnd}`;
  }
  return `Week of  ${dayjs().month(month).format('MMMM')} ${date} - ${dateEnd}, ${year}`;
};

const handle2WeekDisplay = (rangeStart: any, rangeEnd: any) => {
  const date = rangeStart.getDate();
  const dateEnd = rangeEnd.getDate();
  const month = rangeStart.getMonth();
  const monthEnd = rangeEnd.getMonth();
  const year = rangeStart.getFullYear();
  const yearEnd = rangeEnd.getFullYear();
  if (month !== monthEnd) {
    return ` ${dayjs().month(month).format('MMMM')} ${date}, ${year} - ${dayjs()
      .month(monthEnd)
      .format('MMMM')} ${dateEnd}, ${yearEnd}`;
  }
  return ` ${dayjs().month(month).format('MMMM')} ${date} - ${dateEnd}, ${year}`;
};

const handleMonthDisplay = (rangeStart: any, rangeEnd: any) => {
  const date = rangeStart.getDate();
  const dateEnd = rangeEnd.getDate();
  const month = rangeStart.getMonth();
  const monthEnd = rangeEnd.getMonth();
  const year = rangeStart.getFullYear();
  const yearEnd = rangeEnd.getFullYear();
  if (month !== monthEnd) {
    return ` ${dayjs().month(month).format('MMMM')} ${date}, ${year} - ${dayjs()
      .month(monthEnd)
      .format('MMMM')} ${dateEnd}, ${yearEnd}`;
  }
  return ` ${dayjs().month(month).format('MMMM')} ${date} - ${dateEnd}, ${year}`;
};

export const getDisplayDate = (calendarInstance: Calendar, view: Views) => {
  // eslint-disable-next-line @typescript-eslint/ban-ts-comment
  // @ts-ignore
  const calDate = calendarInstance.getDate();
  const day = dayjs().day(calDate.getDay()).format('dddd');

  const date = calDate.getDate();
  const year = calDate.getFullYear();
  const month = dayjs().month(calDate.getMonth()).format('MMMM');

  const rangeStart = calendarInstance.getDateRangeStart();
  const rangeEnd = calendarInstance.getDateRangeEnd();
  let dateText = '';
  switch (view) {
    case 'day':
      dateText = `${day}, ${month} ${date}, ${year}`;
      break;
    case 'week':
      dateText = handleWeekDisplay(rangeStart, rangeEnd);
      break;
    case '2weeks':
      dateText = handle2WeekDisplay(rangeStart, rangeEnd);
      break;
    default:
      dateText = handleMonthDisplay(rangeStart, rangeEnd);
  }

  return dateText;
};

export const getCalendarDayJSDate = (
  // TODO: fix this ts type
  calendarInstance: Calendar,
) => {
  const calDate = calendarInstance.getDate();
  const date = calDate.getDate();
  const year = calDate.getFullYear();
  const month = calDate.getMonth();
  const newDate = new Date(year, month, date);
  return dayjs(newDate);
};

export const formatStartTime = (
  date: Dayjs | null,
  startTime: string,
  endTime: string,
  endDate: Dayjs | null,
) => {
  const startTimeAsDayJs = dayjs(`${date?.format('YYYY-MM-DD')} ${startTime}`);
  const endTimeAsDayJs = dayjs(`${endDate?.format('YYYY-MM-DD')} ${endTime}`);
  const duration = endTimeAsDayJs.diff(startTimeAsDayJs, 'minute');
  const startUTC = startTimeAsDayJs.format();

  return { duration, startUTC };
};

export const tabs: Tab[] = [
  { label: 'Daily', value: 'day' },
  { label: 'Week', value: 'week' },
  { label: '2 Weeks', value: '2weeks' },
  { label: '4 Weeks', value: 'month' },
];

export type MoveDirections = 'next' | 'prev' | 'today';

export const initialEventObject = {
  id: '',
  duration: 0,
  mode: '',
  start_time: '',
  variables: {
    test: 0,
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

export const recurringString = 'recurring';

// TODO: fix arg types
export const getTempEvents = (
  displayEvents: any[] | undefined,
  newValue: unknown,
) => displayEvents?.map((displayEvent) => ({
  ...displayEvent,
  category: newValue === '2weeks' || newValue === 'month' ? ['task'] : undefined,
}));
