/* eslint-disable import/no-extraneous-dependencies */
// TODO: fix lint
import { MuiButton, ThemeType, Typography } from '@flexgen/storybook';
import { Box, ThemeProvider } from '@mui/system';
import isEqual from 'lodash.isequal';
import { useContext, useEffect, useState } from 'react';

import { Configuration } from 'shared/types/dtos/scheduler.dto';
import { NotifContextType, NotifContext } from 'src/contexts/NotifContext';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import { useSchedulerContext } from 'src/pages/Scheduler/Scheduler';
import { schedulerURLS } from 'src/pages/Scheduler/SchedulerComponents/EventScheduler/EventSchedulerHelper';
import FleetManagerConfig from 'src/pages/Scheduler/SchedulerConfiguration/FleetManagerConfig/FleetManager';
import { useTheme } from 'styled-components';
import {
  createMuiTheme,
  schedulerConfigSx,
  headerSx,
  buttonBoxSx,
  siteFleetConfigLabels as labels,
} from './Helpers';
import SiteControllerConfig from './SiteController';

interface SiteFleetConfigProps {
  configured: boolean;
  schedulerType: 'SC' | 'FM' | null;
  setIsLoading: any;
}

const SiteFleetConfig: React.FC<SiteFleetConfigProps> = ({
  configured,
  schedulerType,
  setIsLoading,
}: SiteFleetConfigProps) => {
  const theme = useTheme() as ThemeType;
  const muiTheme = createMuiTheme(theme);
  const boxSx = schedulerConfigSx(theme);
  const buttonSx = buttonBoxSx(theme);
  const [configEdits, setConfigEdits] = useState<Configuration | null | undefined>(null);
  const [saveDisabled, setSaveDisabled] = useState<boolean>(false);
  const notifCtx = useContext<NotifContextType | null>(NotifContext);
  const axiosInstance = useAxiosWebUIInstance();

  const { config, siteName } = useSchedulerContext();

  useEffect(() => {
    setConfigEdits(config);
  }, [config]);

  /** Saves the configuration object and posts it. If cancel then resets the fields */
  const updateConfig = async (action: 'save' | 'cancel') => {
    if (action === 'save') {
      try {
        setIsLoading(true);
        await axiosInstance.post(schedulerURLS.postConfiguration, configEdits);
      } finally {
        notifCtx?.notif('success', labels.notifications.success);
        setIsLoading(false);
      }
    }
    setConfigEdits(config);
  };

  function checkRequiredValues(): boolean {
    let fulfilled = false;
    if (schedulerType === 'SC' && configEdits?.local_schedule?.name) fulfilled = true;
    if (schedulerType === 'FM' && configEdits?.scheduler_type) fulfilled = true;
    return fulfilled;
  }

  return (
    <ThemeProvider theme={muiTheme}>
      <Box sx={boxSx}>
        <Box sx={headerSx(theme)}>
          <Box sx={{ display: 'flex', flexDirection: 'column' }}>
            {configured ? (
              <Typography text={siteName} variant="headingL" />
            ) : (
              <Typography text={labels.pageDescription.unconfigured} variant="headingL" />
            )}
            <Typography
              text={schedulerType === 'SC' ? labels.pageDescription.sc : labels.pageDescription.fm}
              variant="bodyL"
            />
          </Box>
          <Box sx={buttonSx}>
            <MuiButton
              color="inherit"
              label={labels.buttons.cancel}
              onClick={() => updateConfig('cancel')}
              size="small"
              variant="text"
            />
            <MuiButton
              color="primary"
              disabled={isEqual(config, configEdits) || !checkRequiredValues() || saveDisabled}
              label={labels.buttons.save}
              onClick={() => updateConfig('save')}
              size="small"
            />
          </Box>
        </Box>
        {schedulerType === 'SC' && (
          <SiteControllerConfig
            data={configEdits}
            setConfigEdits={setConfigEdits}
            setSaveDisabled={setSaveDisabled}
          />
        )}
        {schedulerType === 'FM' && (
          <FleetManagerConfig
            data={configEdits}
            setConfigEdits={setConfigEdits}
            setSaveDisabled={setSaveDisabled}
          />
        )}
      </Box>
    </ThemeProvider>
  );
};

export default SiteFleetConfig;
