import { Column } from '@flexgen/storybook';
import SortRow from './SortRow/SortRow';
import { SystemStatusObject, SystemStatusRow } from './SystemStatus.types';

export const systemStatusColumns = (
  systemStatusData: SystemStatusObject[],
  generateRowsData: (
    systemStatusData: SystemStatusObject[],
    sortByRow?: keyof SystemStatusObject,
    reverse?: boolean,
  ) => SystemStatusRow[],
): Column[] => [
  {
    id: 'dependencies', label: '', width: 50, align: 'right',
  },
  {
    id: 'service_name',
    label: 'Service Name',
    content: <SortRow label="Service Name" column="serviceName" systemStatusData={systemStatusData} generateRowsData={generateRowsData} />,
  },
  {
    id: 'service_status',
    label: 'Service Status',
    content: <SortRow label="Service Status" column="serviceStatus" systemStatusData={systemStatusData} generateRowsData={generateRowsData} />,
  },
  {
    id: 'connection_status',
    label: 'Connection Status',
    content: <SortRow label="Connection Status" column="connectionStatus" systemStatusData={systemStatusData} generateRowsData={generateRowsData} />,
  },
  {
    id: 'cpu_usage',
    label: 'CPU Usage',
    content: <SortRow label="CPU Usage" column="cpuUsage" systemStatusData={systemStatusData} generateRowsData={generateRowsData} />,
  },
  {
    id: 'memory_usage',
    label: 'Memory Usage',
    content: <SortRow label="Memory Usage" column="memoryUsage" systemStatusData={systemStatusData} generateRowsData={generateRowsData} />,
  },
  {
    id: 'uptime',
    label: 'Time Since Last Restart',
    content: <SortRow label="Time Since Last Restart" column="uptime" systemStatusData={systemStatusData} generateRowsData={generateRowsData} />,
  },
  {
    id: 'last_restart',
    label: 'Last Restart',
    content: <SortRow label="Last Restart" column="lastRestart" systemStatusData={systemStatusData} generateRowsData={generateRowsData} />,
  },
  { id: 'actions', label: 'Actions' },
];

export const toTitleCase = (
  str: string,
) => str.replace(/\w\S*/g, (txt) => txt.charAt(0).toUpperCase() + txt.substring(1).toLowerCase());

export const formatServiceName = (
  serviceName: string,
) => toTitleCase(serviceName.replace(/_/g, ' '));
