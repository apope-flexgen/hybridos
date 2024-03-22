import {
  Box, Tab, Tabs, ThemeType,
} from '@flexgen/storybook';
import { useState } from 'react';
import { useAppContext } from 'src/App/App';
import { useTheme } from 'styled-components';
import AlertManagement from './AlertManagement/AlertManagement';
import Alerts from './Alerts/Alerts';
import Events from './Events/Events';
import ResolvedAlerts from './ResolvedAlerts/ResolvedAlerts';
import { ActivityLogTabs, activityLogTabs } from './activitiyLog.helpers';
import { mainContentBoxSx, tabBoxSx, eventsBoxSx } from './activityLog.styles';

const ActivityLog: React.FunctionComponent = () => {
  const [selectedTab, setSelectedTab] = useState(ActivityLogTabs.Events);
  const theme = useTheme() as ThemeType;
  const mainBoxSx = mainContentBoxSx(theme);

  const {
    siteConfiguration,
  } = useAppContext();

  return (
    <Box sx={{ width: '100%' }}>
      {
        siteConfiguration?.events
        && !siteConfiguration?.alerting
          ? <Box sx={eventsBoxSx(theme)}><Events /></Box>
          : (
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
                  />
                ))}
              </Tabs>
              <Box sx={tabBoxSx}>
                {
                    selectedTab === ActivityLogTabs.Events
                    && <Events />
                    }
                {
                    selectedTab === ActivityLogTabs.Alerts
                    && <Alerts />
                    }
                {
                    selectedTab === ActivityLogTabs.ResolvedAlerts
                    && <ResolvedAlerts />
                    }
                {
                    selectedTab === ActivityLogTabs.AlertManagement
                    && <AlertManagement />
              }
              </Box>
            </Box>
          )
      }
    </Box>
  );
};

export default ActivityLog;
