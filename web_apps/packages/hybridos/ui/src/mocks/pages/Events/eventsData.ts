import { SeverityType } from '@flexgen/storybook';
import dayjs from 'dayjs';
import { Event } from 'shared/types/dtos/events.dto';

const now = dayjs();

const eventsDataTemplate = [
  {
    severity: 'Alarm',
    source: 'Assets',
    message: 'Process Scheduler is hung or dead',
  },
  {
    severity: 'Fault',
    source: 'Modbus Client',
    message: 'Site_Manager primary_controller status changed to: true or dead',
  },
  {
    severity: 'Info',
    source: 'Modbus Server',
    message: 'Site_Manager primary_controller status changed to: true or dead',
  },
  {
    severity: 'Status',
    source: 'Site',
    message: 'Process Scheduler is hung or dead',
  },
];

const eventsData: Event[] = [];

for (let i = 0; i < 40000; i += 1) {
  const newTimeStamp = now.subtract(i * 10, 'minute').format('YYYY/MM/DD HH:mm:ss');
  const randomIndexForSeverity = Math.floor(Math.random() * 4);
  const randomIndexForSource = Math.floor(Math.random() * 4);
  const randomIndexForMessage = Math.floor(Math.random() * 4);
  const newDataObject: Event = {
    id: i.toString(),
    severity: eventsDataTemplate[randomIndexForSeverity].severity as SeverityType,
    source: eventsDataTemplate[randomIndexForSource].source,
    message: eventsDataTemplate[randomIndexForMessage].message,
    timestamp: newTimeStamp,
  };
  eventsData.push(newDataObject);
}

export default eventsData;
