import {
  Box, Tab, Tabs, ThemeType, Typography,
} from '@flexgen/storybook';
import { useEffect, useState } from 'react';
import { useAppContext } from 'src/App/App';
import useAxiosWebUIInstance from 'src/hooks/useAxios';

import { initialActiveAlertsFilters } from 'src/pages/ActivityLog/Alerts/alerts.helpers';
import { useTheme } from 'styled-components';
import AlertManagement from './AlertManagement/AlertManagement';
import Alerts from './Alerts/Alerts';
import Events from './Events/Events';
import ResolvedAlerts from './ResolvedAlerts/ResolvedAlerts';
import { ActivityLogTabs, activityLogTabs } from './activityLog.helpers';
import {
  mainContentBoxSx, tabBoxSx, eventsBoxSx, alertBubbleSx,
} from './activityLog.styles';

const ActivityLog: React.FunctionComponent = () => {
  const [selectedTab, setSelectedTab] = useState(ActivityLogTabs.Events);
  const [totalActiveAlertCount, setTotalActiveAlertCount] = useState<string>('');

  const theme = useTheme() as ThemeType;
  const mainBoxSx = mainContentBoxSx(theme);
  const axiosInstance = useAxiosWebUIInstance();

  const { siteConfiguration } = useAppContext();

  const getInitialAlertCount = async () => {
    const filtersArray = Object.keys(initialActiveAlertsFilters)
      .filter((key) => key !== null && key !== undefined)
      .map((key: string) => [key, initialActiveAlertsFilters[key].toString()]);

    const filtersWithAmpersand = new URLSearchParams(filtersArray);
    const activeAlertsURL = `/alerts/active?${filtersWithAmpersand}`;

    axiosInstance.get(activeAlertsURL).then((res) => {
      setTotalActiveAlertCount(res.data.count.toString());
    });
  };

  useEffect(() => {
    getInitialAlertCount();
  }, []);

  return (
    <Box sx={{ width: '100%' }}>
      {siteConfiguration?.events && !siteConfiguration?.alerting ? (
        <Box sx={eventsBoxSx(theme)}>
          <Events />
        </Box>
      ) : (
        <Box sx={mainBoxSx}>
          <Tabs
            onChange={(_, newValue: any) => {
              setSelectedTab(newValue);
            }}
            value={selectedTab}
          >
            {activityLogTabs.map((tab) => (
              <Tab
                key={tab.value}
                label={tab.label}
                value={tab.value}
                iconPosition="end"
                icon={
                  tab.value === ActivityLogTabs.Alerts
                  && totalActiveAlertCount
                  && totalActiveAlertCount !== '0' ? (
                    <Box sx={alertBubbleSx(theme)}>
                      <Typography text={totalActiveAlertCount} variant="bodySBold" color="color" />
                    </Box>
                    ) : (
                      <></>
                    )
                }
              />
            ))}
          </Tabs>
          <Box sx={tabBoxSx}>
            {selectedTab === ActivityLogTabs.Events && <Events />}
            {selectedTab === ActivityLogTabs.Alerts && (
              <Alerts setTotalActiveAlertCount={setTotalActiveAlertCount} />
            )}
            {selectedTab === ActivityLogTabs.ResolvedAlerts && <ResolvedAlerts />}
            {selectedTab === ActivityLogTabs.AlertManagement && <AlertManagement />}
          </Box>
        </Box>
      )}
    </Box>
  );
};

export default ActivityLog;
