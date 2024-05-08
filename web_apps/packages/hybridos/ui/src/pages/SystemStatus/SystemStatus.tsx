import { Box, ThemeType, PageLoadingIndicator } from '@flexgen/storybook';
import { useCallback, useEffect, useState } from 'react';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import QueryService from 'src/services/QueryService';
import { useTheme } from 'styled-components';

import { SYSTEM_STATUS_URL } from './SystemStatus.constants';
import { mainContentBoxSx, headerBoxSx, tableBoxSx } from './SystemStatus.styles';
import { SystemStatusObject } from './SystemStatus.types';
import SystemStatusHeader from './SystemStatusHeader/SystemStatusHeader';
import SystemStatusTable from './SystemStatusTable/SystemStatusTable';

const SystemStatus: React.FunctionComponent = () => {
  const [systemStatusData, setSystemStatusData] = useState<SystemStatusObject[]>([]);
  const [summarySystemData, setSummarySystemData] = useState<SystemStatusObject>({});
  const [displayData, setDisplayData] = useState<SystemStatusObject[]>([]);
  const [isLoading, setIsLoading] = useState(true);

  const theme = useTheme() as ThemeType;
  const mainBoxSx = mainContentBoxSx(theme);
  const axiosInstance = useAxiosWebUIInstance();

  const handleSystemStatusData = useCallback((newDataFromSocket: SystemStatusObject) => {
    if (!newDataFromSocket) return;

    if (newDataFromSocket.serviceName === 'summary') {
      setSummarySystemData(newDataFromSocket);
    } else {
      setSystemStatusData((prevState) => {
        const tempArray = prevState;
        const newServiceName = newDataFromSocket.serviceName;
        const indexOfService = tempArray.findIndex((item) => item.serviceName === newServiceName);
        // if service already in table, replace its entry with new data
        if (indexOfService !== -1) {
          tempArray[indexOfService] = newDataFromSocket;
          return [...tempArray];
        }
        // else, just append new service to the end of the data
        return [...prevState, newDataFromSocket];
      });
    }
  }, []);

  useEffect(() => {
    QueryService.getSystemStatus(handleSystemStatusData);

    return () => {
      QueryService.cleanupSocket();
    };
  }, []);

  const getInitialData = async () => {
    axiosInstance.get(SYSTEM_STATUS_URL).then((res) => {
      setSystemStatusData(() => {
        const summaryData = res.data.find(
          (entry: SystemStatusObject) => entry.serviceName === 'summary',
        );
        if (summaryData) setSummarySystemData(summaryData);

        return res.data.filter(
          (entry: SystemStatusObject) => entry.serviceName !== 'summary',
        ) as SystemStatusObject[];
      });
      setIsLoading(false);
    });
  };

  // initial GET request to populate data for page
  useEffect(() => {
    getInitialData();
  }, []);

  return (
    <Box sx={mainBoxSx}>
      <Box sx={headerBoxSx}>
        <SystemStatusHeader
          systemStatusData={systemStatusData}
          summarySystemData={summarySystemData}
          setDisplayData={setDisplayData}
        />
      </Box>
      <Box sx={tableBoxSx}>
        <SystemStatusTable systemStatusData={displayData} />
      </Box>
      <PageLoadingIndicator isLoading={isLoading} type="primary" />
    </Box>
  );
};

export default SystemStatus;
