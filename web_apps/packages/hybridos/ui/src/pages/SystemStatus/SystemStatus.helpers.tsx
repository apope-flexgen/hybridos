import {
  Box, Column, IconButton, Typography,
} from '@flexgen/storybook';
import { useState } from 'react';
import { sortedTableHeaderSx } from './SystemStatus.styles';
import { SystemStatusObject, SystemStatusRow } from './SystemStatus.types';

export const SortRow = (
  label: string,
  column: keyof SystemStatusObject,
  systemStatusData: SystemStatusObject[],
  generateRowsData: (
    systemStatusData: SystemStatusObject[],
    sortByRow?: keyof SystemStatusObject,
    reverse?: boolean,
  ) => SystemStatusRow[],
) => {
  const [reverseOrder, setReverseOrder] = useState<boolean>(false);
  const handleSort = () => {
    generateRowsData(systemStatusData, column, reverseOrder);
    setReverseOrder(!reverseOrder);
  };
  return (
    <Box sx={sortedTableHeaderSx}>
      <Typography text={label} variant="tableHeader" />
      <IconButton
        icon={reverseOrder ? 'ArrowDown' : 'ArrowUp'}
        onClick={handleSort}
        size="small"
      />
    </Box>
  );
};

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
  { id: 'service_name', label: 'Service Name', content: SortRow('Service Name', 'serviceName', systemStatusData, generateRowsData) },
  { id: 'service_status', label: 'Service Status', content: SortRow('Service Status', 'serviceStatus', systemStatusData, generateRowsData) },
  { id: 'connection_status', label: 'Connection Status', content: SortRow('Connection Status', 'connectionStatus', systemStatusData, generateRowsData) },
  { id: 'cpu_usage', label: 'CPU Usage', content: SortRow('CPU Usage', 'cpuUsage', systemStatusData, generateRowsData) },
  { id: 'memory_usage', label: 'Memory Usage', content: SortRow('Memory Usage', 'memoryUsage', systemStatusData, generateRowsData) },
  { id: 'uptime', label: 'Time Since Last Restart', content: SortRow('Time Since Last Restart', 'uptime', systemStatusData, generateRowsData) },
  { id: 'last_restart', label: 'Last Restart', content: SortRow('Last Restart', 'lastRestart', systemStatusData, generateRowsData) },
  { id: 'actions', label: 'Actions' },
];

export const toTitleCase = (
  str: string,
) => str.replace(/\w\S*/g, (txt) => txt.charAt(0).toUpperCase() + txt.substring(1).toLowerCase());

export const formatServiceName = (
  serviceName: string,
) => toTitleCase(serviceName.replace(/_/g, ' '));
