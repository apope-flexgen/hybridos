/* eslint-disable max-lines */
import {
  Box, Chip, Icon, IconButton, Tooltip, Typography,
} from '@flexgen/storybook';
import dayjs from 'dayjs';
import { useState } from 'react';
import { statusColorMapping } from 'src/pages/SystemStatus/SystemStatus.constants';
import { formatServiceName, toTitleCase } from 'src/pages/SystemStatus/SystemStatus.helpers';
import { iconSize, actionBoxSx } from 'src/pages/SystemStatus/SystemStatus.styles';
import { SystemStatusObject, SystemStatusRow } from 'src/pages/SystemStatus/SystemStatus.types';

const useGenerateSystemStatusTable = () => {
  const [results, setResults] = useState<SystemStatusRow[]>([]);

  const generateActionButtons = (
    actionArray: string[],
    serviceName: string,
  ) => {
    const handleOnClick = (action: 'start' | 'stop' | 'restart') => {
      const serviceActionURL = `cops/stats/${serviceName}/${action}`;
      // TODO: send request to backend to actually complete request
      console.log(serviceActionURL);
    };
    const emptyIconBox = <Box sx={iconSize} />;
    return (
      <Box sx={actionBoxSx}>
        {
          actionArray.includes('start')
            ? (
              <Tooltip title="Start" arrow placement="bottom">
                <IconButton icon="StartSystem" color="success" size="small" onClick={() => handleOnClick('start')} />
              </Tooltip>
            )
            : emptyIconBox
        }
        {
          actionArray.includes('stop')
            ? (
              <Tooltip title="Stop" arrow placement="bottom">
                <IconButton icon="Cancel" color="error" size="small" onClick={() => handleOnClick('stop')} />
              </Tooltip>
            )
            : emptyIconBox
        }
        {
          actionArray.includes('restart')
            ? (
              <Tooltip title="Restart" arrow placement="bottom">
                <IconButton icon="RestartSystem" color="primary" size="small" onClick={() => handleOnClick('restart')} />
              </Tooltip>
            )
            : emptyIconBox
        }
      </Box>
    );
  };

  const generateDependencies = (
    dependencies: string[],
  ) => {
    if (dependencies && dependencies.length > 0) {
      let dependenciesMessage = 'This service has dependent services - ';
      dependencies.forEach((dependency, index) => {
        const dependencyInTitleCase = toTitleCase(dependency.replace(/_/g, ' '));
        if (index === dependencies.length - 1) dependenciesMessage += dependencyInTitleCase;
        else dependenciesMessage += `${dependencyInTitleCase}, `;
      });
      return (
        <Tooltip title={dependenciesMessage} arrow placement="bottom">
          <Icon src="Connected" />
        </Tooltip>
      );
    }
    return null;
  };

  const generateStatusTag = (
    serviceStatus: string,
  ) => (
    <Chip
      label={toTitleCase(serviceStatus)}
      icon="Circle"
      iconColor={statusColorMapping[serviceStatus.split(' ')[0].toLowerCase()] || 'primary'}
      variant="outlined"
      size="small"
      borderStyle="squared"
    />
  );

  const generateConnectionStatus = (
    connectionStatus: string,
  ) => {
    if (connectionStatus.toLowerCase() === 'online') {
      return (
        <Box sx={actionBoxSx}>
          <Icon src="NetworkConnected" color="success" size="small" />
          <Typography text="Online" color="success" variant="bodyMBold" />
        </Box>
      );
    }
    return (
      <Box sx={actionBoxSx}>
        <Icon src="NetworkDisconnected" color="error" size="small" />
        <Typography text="Offline" color="error" variant="bodyMBold" />
      </Box>
    );
  };

  const generateRowsData = (
    systemStatusData: SystemStatusObject[],
    sortByRow?: keyof SystemStatusObject,
    reverseOrder?: boolean,
  ) => {
    const sortedArray = systemStatusData;

    if (sortByRow) {
      const rowIndex = sortByRow;
      sortedArray.sort(
        (objectA, objectB) => {
          if (!reverseOrder) return `${objectA[rowIndex]}`.localeCompare(`${objectB[rowIndex]}`, 'en', { numeric: true });
          return `${objectB[rowIndex]}`.localeCompare(`${objectA[rowIndex]}`, 'en', { numeric: true });
        },
      );
    }

    const returnData: SystemStatusRow[] = sortedArray.map((serviceData) => ({
      id: serviceData.serviceName || '',
      dependencies: generateDependencies(serviceData.dependencies || []),
      service_name: formatServiceName(serviceData.serviceName || ''),
      service_status: serviceData.serviceStatus ? generateStatusTag(serviceData.serviceStatus) : '-',
      connection_status: serviceData.connectionStatus ? generateConnectionStatus(serviceData.connectionStatus) : '-',
      cpu_usage: serviceData.cpuUsage ? `${serviceData.cpuUsage}%` : '-',
      memory_usage: (serviceData.memoryUsage && serviceData.memoryUsage !== -1) ? `${serviceData.memoryUsage}%` : '-',
      uptime: (serviceData.uptime && serviceData.uptime !== -1) ? serviceData.uptime : '-',
      last_restart: serviceData.lastRestart ? dayjs(serviceData.lastRestart).format() : '-',
      actions: generateActionButtons(serviceData.actions || [], serviceData.serviceName || ''),
    }));

    setResults(returnData);
    return returnData;
  };

  return { results, generateRowsData };
};

export default useGenerateSystemStatusTable;
