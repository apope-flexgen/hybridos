/* eslint-disable max-len */
import {
  Box, DatePicker, DualTimePicker, Select,
} from '@flexgen/storybook';
import { SelectChangeEvent } from '@mui/material';
import dayjs, { Dayjs } from 'dayjs';
import { FC, useState } from 'react';
import { EventsRequestParams } from 'shared/types/dtos/events.dto';
import { datePickerSx } from 'src/pages/ActivityLog/Events/Styles';
import {
  checkIfStartBeforeEnd,
  datePickerLabel,
  handleDateChange,
  handleEndTimeChange,
  handleStartTimeChange,
  handleTimeFrameChange,
  timeFrameLabel,
  timeFrameOptions,
} from './EventDateTime-helpers';

interface EventsDateTimeProps {
  filters: EventsRequestParams;
  setFilters: (value: React.SetStateAction<EventsRequestParams>) => void;
}

const EventDateTime: FC<EventsDateTimeProps> = ({ filters, setFilters }: EventsDateTimeProps) => {
  const [date, setDateValue] = useState<Dayjs | null>(dayjs());
  const [startTime, setStartTime] = useState('');
  const [endTime, setEndTime] = useState('');
  const [timeFrame, setTimeFrame] = useState('Last Ten Minutes');

  return (
    <>
      <Box sx={datePickerSx}>
        <DatePicker
          label={datePickerLabel}
          onChange={(newDate: dayjs.Dayjs | null) => {
            setDateValue(newDate);
            handleDateChange(newDate, setTimeFrame, filters, setFilters);
          }}
          size="small"
          value={date}
        />
      </Box>
      <Select
        label={timeFrameLabel}
        menuItems={timeFrameOptions}
        onChange={(event: SelectChangeEvent<string>) => handleTimeFrameChange(
          event,
          setTimeFrame,
          setDateValue,
          startTime,
          endTime,
          filters,
          setFilters,
        )}
        value={timeFrame}
      />
      <DualTimePicker
        disabled={timeFrame !== 'Custom' || date === null}
        error={checkIfStartBeforeEnd(startTime, endTime)}
        onEndChange={(event: SelectChangeEvent<string>) => handleEndTimeChange(event, setEndTime, date, startTime, filters, setFilters)}
        onStartChange={(event: SelectChangeEvent<string>) => handleStartTimeChange(event, setStartTime, date, filters, setFilters)}
        values={[startTime, endTime]}
      />
    </>
  );
};

export default EventDateTime;
