import dayjs from 'dayjs';
import { SchedulerEvent } from 'shared/types/dtos/scheduler.dto';

const now = dayjs();
const schedulerEventsData: SchedulerEvent[] = [
  {
    id: Math.floor(Math.random() * 1000000000).toString(),
    duration: 90,
    mode: 'charging',
    start_time: now.minute(30).format(),
    variables: {
      charge_cmd: 3000,
      active_pwr: 2500,
    },
  },
  {
    id: Math.floor(Math.random() * 1000000000).toString(),
    duration: 90,
    mode: 'discharging',
    start_time: now.minute(30).add(2, 'day').format(),
    variables: {},
  },
  {
    id: Math.floor(Math.random() * 1000000000).toString(),
    duration: 90,
    mode: 'maintenance',
    start_time: now.minute(60).add(5, 'day').format(),
    variables: {
      reactive_pwr: 3000,
    },
    repeat: {
      start: now.minute(60).add(5, 'day').format(),
      id: Math.floor(Math.random() * 1000000000).toString(),
      cycle: 'day',
      frequency: 4,
    },
  },
  {
    id: Math.floor(Math.random() * 1000000000).toString(),
    duration: 90,
    mode: 'charging',
    start_time: now.minute(90).add(8, 'day').format(),
    variables: {
      charge_cmd: 3000,
      active_pwr: 1500,
    },
    repeat: {
      start: now.minute(90).add(8, 'day').format(),
      id: Math.floor(Math.random() * 1000000000).toString(),
      cycle: 'day',
      frequency: 2,
      end_count: 9,
    },
  },
  {
    id: Math.floor(Math.random() * 1000000000).toString(),
    duration: 90,
    mode: 'discharging',
    start_time: now.minute(120).add(1, 'week').day(1).format(),
    variables: {},
    repeat: {
      start: now.minute(120).add(1, 'week').day(1).format(),
      id: Math.floor(Math.random() * 1000000000).toString(),
      cycle: 'week',
      frequency: 2,
      day_mask: 42,
      end_time: now.minute(120).add(6, 'week').day(1).format(),
    },
  },
];

export default schedulerEventsData;
