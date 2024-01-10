/* eslint-disable max-lines */
import {
  Box, Chip, Icon, IconButton, Tooltip, Typography,
} from '@flexgen/storybook';

import { useContext, useState } from 'react';
import { ConnectionStatus } from 'shared/types/dtos/systemStatus.dto';
import { NotifContextType, NotifContext } from 'src/contexts/NotifContext';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import { SYSTEM_STATUS_URL, statusColorMapping } from 'src/pages/SystemStatus/SystemStatus.constants';
import { toTitleCase } from 'src/pages/SystemStatus/SystemStatus.helpers';
import { iconSize, actionBoxSx } from 'src/pages/SystemStatus/SystemStatus.styles';
import {
  ServiceActionObject, ServiceActionType, SystemStatusObject, SystemStatusRow,
} from 'src/pages/SystemStatus/SystemStatus.types';

const useGenerateSystemStatusTable = () => {
  const [results, setResults] = useState<SystemStatusRow[]>([]);
  const axiosInstance = useAxiosWebUIInstance();
  const notifCtx = useContext<NotifContextType | null>(NotifContext);

  const generateActionButtons = (
    actionArray: ServiceActionObject[],
    serviceName: string,
  ) => {
    const handleOnClick = async (action: ServiceActionType) => {
      const serviceActionURL = `${SYSTEM_STATUS_URL}/${serviceName}/${action}`;
      axiosInstance.put(serviceActionURL).then((res) => {
        if (res.data.body === 'done') notifCtx?.notif('success', `Succesfully completed ${action} on ${serviceName}`);
        else notifCtx?.notif('error', `Error completing ${action} on ${serviceName} - ${res.data.body}`);
      });
    };

    const availableActions = actionArray.map((serviceAction) => serviceAction.action);
    let enabledStatuses: { [key: string ]: boolean } = {};
    actionArray.forEach((serviceAction) => {
      enabledStatuses = { ...enabledStatuses, [serviceAction.action]: serviceAction.enabled };
    });

    const emptyIconBox = <Box sx={iconSize} />;
    return (
      <Box sx={actionBoxSx}>
        {
          availableActions.includes('start')
            ? (
              <Tooltip title="Start" arrow placement="bottom">
                <IconButton
                  icon="StartSystem"
                  color="success"
                  size="small"
                  disabled={!enabledStatuses.start}
                  onClick={() => handleOnClick('start')}
                />
              </Tooltip>
            )
            : emptyIconBox
        }
        {
          availableActions.includes('stop')
            ? (
              <Tooltip title="Stop" arrow placement="bottom">
                <IconButton
                  icon="Cancel"
                  color="error"
                  size="small"
                  disabled={!enabledStatuses.stop}
                  onClick={() => handleOnClick('stop')}
                />
              </Tooltip>
            )
            : emptyIconBox
        }
        {
          availableActions.includes('restart')
            ? (
              <Tooltip title="Restart" arrow placement="bottom">
                <IconButton
                  icon="RestartSystem"
                  color="primary"
                  disabled={!enabledStatuses.restart}
                  size="small"
                  onClick={() => handleOnClick('restart')}
                />
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
      const dependenciesList = dependencies.map((dependency) => dependency.replace('.service', '')).join(', ');
      const dependenciesMessage = `This service is dependent upon the following services: ${dependenciesList}`;
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
    connectionStatus: ConnectionStatus | null,
  ) => {
    if (connectionStatus === ConnectionStatus.Online) {
      return (
        <Box sx={actionBoxSx}>
          <Icon src="NetworkConnected" color="success" size="small" />
          <Typography text="Online" color="success" variant="bodyMBold" />
        </Box>
      );
    }
    if (connectionStatus === ConnectionStatus.Offline) {
      return (
        <Box sx={actionBoxSx}>
          <Icon src="NetworkDisconnected" color="error" size="small" />
          <Typography text="Offline" color="error" variant="bodyMBold" />
        </Box>
      );
    }
    return (
      <Box sx={actionBoxSx}>
        <Icon src="Remove" color="info" size="small" />
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
      service_name: serviceData.serviceName || '',
      service_status: serviceData.serviceStatus ? generateStatusTag(serviceData.serviceStatus) : '-',
      connection_status: generateConnectionStatus(serviceData.connectionStatus || null),
      cpu_usage: (serviceData.cpuUsage !== -1) ? `${serviceData.cpuUsage}%` : '-',
      memory_usage: (serviceData.memoryUsage !== -1) ? `${serviceData.memoryUsage}%` : '-',
      uptime: (serviceData.uptime) ? serviceData.uptime : '-',
      last_restart: serviceData.lastRestart || '-',
      actions: generateActionButtons(serviceData.actions || [], serviceData.serviceName || ''),
    }));

    setResults(returnData);
    return returnData;
  };

  return { results, generateRowsData };
};

export default useGenerateSystemStatusTable;
