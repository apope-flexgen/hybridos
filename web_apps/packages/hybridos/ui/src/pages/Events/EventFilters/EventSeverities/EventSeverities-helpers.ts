import { SeverityProperties } from './EventSeveritiesTypes';

export const initialSeverities = {
  Alarm: true,
  Fault: true,
  Info: false,
  Status: false,
};

export const severitiesProperties: SeverityProperties = {
  Alarm: {
    color: 'warning',
    label: 'Alarm',
  },
  Fault: {
    color: 'error',
    label: 'Fault',
  },
  Info: {
    label: 'Info',
  },
  Status: {
    label: 'Status',
  },
};
