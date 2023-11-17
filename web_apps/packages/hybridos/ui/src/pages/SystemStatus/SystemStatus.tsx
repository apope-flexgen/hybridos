import {
  Box, ThemeType,
} from '@flexgen/storybook';
import { useEffect, useState } from 'react';
import QueryService from 'src/services/QueryService';
import { useTheme } from 'styled-components';

import { mainContentBoxSx, headerBoxSx, tableBoxSx } from './SystemStatus.styles';
import { SystemStatusObject } from './SystemStatus.types';
import SystemStatusHeader from './SystemStatusHeader/SystemStatusHeader';
import SystemStatusTable from './SystemStatusTable/SystemStatusTable';

const SystemStatus: React.FunctionComponent = () => {
  const [systemStatusData, setSystemStatusData] = useState<SystemStatusObject[]>([]);
  const [displayData, setDisplayData] = useState<SystemStatusObject[]>([]);

  const theme = useTheme() as ThemeType;
  const mainBoxSx = mainContentBoxSx(theme);

  const handleSystemStatusData = (newDataFromSocket: SystemStatusObject) => {
    setSystemStatusData((prevState) => {
      const tempArray = prevState;
      const newServiceName = newDataFromSocket.serviceName;
      const indexOfService = tempArray.findIndex((item) => item.serviceName === newServiceName);
      // if service already in table, replace it's entry with new data
      if (indexOfService !== -1) {
        tempArray[indexOfService] = newDataFromSocket;
        return [...tempArray];
      }
      // else, just append new service to the end of the data
      return [...prevState, newDataFromSocket];
    });
  };

  useEffect(() => {
    QueryService.getSystemStatus(handleSystemStatusData);

    return () => {
      QueryService.cleanupSocket();
    };
  }, []);

  return (
    <Box sx={mainBoxSx}>
      <Box sx={headerBoxSx}>
        <SystemStatusHeader systemStatusData={systemStatusData} setDisplayData={setDisplayData} />
      </Box>
      <Box sx={tableBoxSx}>
        <SystemStatusTable systemStatusData={displayData} />
      </Box>
    </Box>
  );
};

export default SystemStatus;
