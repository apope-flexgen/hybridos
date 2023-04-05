import { SelectChangeEvent } from '@mui/material';
import dayjs, { Dayjs } from 'dayjs';
import { EventsRequestParams } from 'shared/types/dtos/events.dto';

export const timeFrameOptions = ['Last Ten Minutes', 'Last Thirty Minutes', 'Custom'];
export const timeFrameLabel = 'Time Frame';
export const datePickerLabel = 'Select Date';

export const handleEndTimeChange = (
  event: SelectChangeEvent,
  setEndTime: React.Dispatch<React.SetStateAction<string>>,
  date: dayjs.Dayjs | null,
  startTime: string,
  filters: EventsRequestParams,
  setFilters: (value: React.SetStateAction<EventsRequestParams>) => void,
) => {
  const newEndTime = event.target.value;
  setEndTime(newEndTime);
  const endTimeDate = `${date?.format('YYYY/MM/DD ')} ${newEndTime}:00`;
  const startTimeDate = `${date?.format('YYYY/MM/DD ')} ${startTime}:00`;

  const startTimeAsDayJs = dayjs(startTimeDate, 'YYYY-MM-DD HH:mm:ss');
  const endTimeAsDayJs = dayjs(endTimeDate, 'YYYY-MM-DD HH:mm:ss');
  if (startTime === '' || endTimeAsDayJs.isAfter(startTimeAsDayJs)) {
    setFilters({ ...filters, endTime: endTimeDate });
  }
};

export const handleStartTimeChange = (
  event: SelectChangeEvent,
  setStartTime: React.Dispatch<React.SetStateAction<string>>,
  date: dayjs.Dayjs | null,
  filters: EventsRequestParams,
  setFilters: (value: React.SetStateAction<EventsRequestParams>) => void,
) => {
  const newStartTime = event.target.value;
  setStartTime(newStartTime);

  setFilters({
    ...filters,
    startTime: `${date?.format('YYYY/MM/DD ')} ${newStartTime}:00`,
  });
};

export const checkIfStartBeforeEnd = (startTime: string, endTime: string): boolean => {
  if (startTime === '' || endTime === '') {
    return false;
  }

  return startTime >= endTime;
};

export const handleDateChange = (
  newDate: Dayjs | null,
  setTimeFrame: React.Dispatch<React.SetStateAction<string>>,
  startTime: string,
  filters: EventsRequestParams,
  setFilters: (value: React.SetStateAction<EventsRequestParams>) => void,
) => {
  const now = dayjs();
  if (newDate !== now) setTimeFrame('Custom');

  const currentStartTime = startTime === '' ? '00:00:00' : dayjs(startTime).format('HH:mm:ss');

  let currentEndTime;
  if (filters.endTime === '') currentEndTime = '00:00:00';
  else if (dayjs(filters.endTime).format('HH:mm') === dayjs(now).format('HH:mm')) {
    currentEndTime = '23:59:59';
  } else currentEndTime = dayjs(filters.endTime).format('HH:mm:ss');

  const newStartDate = newDate ? newDate.format('YYYY/MM/DD') : filters.startTime;

  setFilters({
    ...filters,
    startTime: `${newStartDate} ${currentStartTime}`,
    endTime: `${newStartDate} ${currentEndTime}`,
  });
};

export const handleTimeFrameChange = (
  event: SelectChangeEvent,
  setTimeFrame: React.Dispatch<React.SetStateAction<string>>,
  setDateValue: React.Dispatch<React.SetStateAction<dayjs.Dayjs | null>>,
  startTime: string,
  endTime: string,
  filters: EventsRequestParams,
  setFilters: (value: React.SetStateAction<EventsRequestParams>) => void,
) => {
  const now = dayjs();

  const newTimeFrame = event.target.value as string;
  setTimeFrame(newTimeFrame);
  let newStartTime;
  let newEndTime;

  if (newTimeFrame === 'Last Thirty Minutes') {
    setDateValue(now);

    newStartTime = now.subtract(30, 'minute').format('YYYY/MM/DD HH:mm:ss');
    newEndTime = now.format('YYYY/MM/DD HH:mm:ss');
  } else if (newTimeFrame === 'Last Ten Minutes') {
    setDateValue(now);

    newStartTime = now.subtract(10, 'minute').format('YYYY/MM/DD HH:mm:ss');
    newEndTime = now.format('YYYY/MM/DD HH:mm:ss');
  } else {
    newStartTime = startTime;
    newEndTime = endTime;
  }
  setFilters({
    ...filters,
    startTime: newStartTime,
    endTime: newEndTime,
  });
};
