// TODO: fix lint
/* eslint-disable max-lines */
import {
  ThemeType, Switch, CardRow, TextField, NumericInput,
} from '@flexgen/storybook';
import { Box, Typography } from '@mui/material';
import React from 'react';
import { Configuration } from 'shared/types/dtos/scheduler.dto';
import { useTheme } from 'styled-components';
import { siteFleetConfigLabels as labels, siteControllerLabels as SClabels, switchBoxSx } from './Helpers';
import SCADASettings from './SCADASettings';

interface SiteControllerConfigProps {
  data?: Configuration | null
  setConfigEdits: React.Dispatch<React.SetStateAction<Configuration | null | undefined>>
}

const SiteControllerConfig: React.FC<SiteControllerConfigProps> = ({
  data,
  setConfigEdits,
}: SiteControllerConfigProps) => {
  const theme = useTheme() as ThemeType;
  const [showSCADA, setShowSCADA] = React.useState(false);

  return (
    <Box>
      <CardRow alignItems="center">
        <Typography
          sx={{
            marginTop: theme.fgb.siteFleetConfig.sizing.typographyMarginTop,
          }}
          variant="h2"
        >
          {SClabels.settings}
        </Typography>
      </CardRow>
      <CardRow alignItems="center">
        <Typography
          sx={{ width: theme.fgb.siteFleetConfig.sizing.typographyWidth }}
          variant="body2"
        >
          {SClabels.siteName}
        </Typography>
        <Box sx={{ width: '500px' }}>
          <TextField
            color={data?.local_schedule?.name.length !== 0 ? 'primary' : 'error'}
            fullWidth
            helperText={data?.local_schedule?.name ? undefined : labels.helperText.name}
            label={SClabels.siteName}
            onChange={(event) => setConfigEdits((prevState: any) => ({
              ...prevState,
              local_schedule: {
                ...prevState.local_schedule,
                name: event.target.value,
              },
            }))}
            size="small"
            value={data?.local_schedule?.name || ''}
          />
        </Box>
      </CardRow>
      <CardRow alignItems="center">
        <Typography
          sx={{ width: theme.fgb.siteFleetConfig.sizing.typographyWidth }}
          variant="body2"
        >
          {SClabels.clothedSetpoints.main}
        </Typography>
        <Box sx={switchBoxSx}>
          <Switch
            color="primary"
            label={SClabels.clothedSetpoints.switch}
            labelPlacement="right"
            onChange={(event) => setConfigEdits((prevState: any) => ({
              ...prevState,
              local_schedule: {
                ...prevState.local_schedule,
                clothed_setpoints: !prevState.local_schedule.clothed_setpoints || event,
              },
            }))}
            value={data?.local_schedule?.clothed_setpoints || false}
          />
        </Box>
      </CardRow>
      <CardRow alignItems="center">
        <Typography
          sx={{ width: theme.fgb.siteFleetConfig.sizing.typographyWidth }}
          variant="body2"
        >
          {SClabels.setpointEnforcement.main}
        </Typography>
        <Box sx={switchBoxSx}>
          <Switch
            color="primary"
            label={SClabels.setpointEnforcement.switch}
            labelPlacement="right"
            onChange={(event) => setConfigEdits((prevState: any) => ({
              ...prevState,
              local_schedule: {
                ...prevState.local_schedule,
                setpoint_enforcement: {
                  ...prevState.local_schedule.setpoint_enforcement,
                  enabled: !prevState.local_schedule?.setpoint_enforcement?.enabled || event,
                },
              },
            }))}
            value={data?.local_schedule?.setpoint_enforcement?.enabled || false}
          />
        </Box>
        <NumericInput
          label={SClabels.setpointEnforcement.textField}
          endComponentAdorment={ <></> }
          onChange={(event) => setConfigEdits((prevState: any) => ({
            ...prevState,
            local_schedule: {
              ...prevState.local_schedule,
              setpoint_enforcement: {
                ...prevState.local_schedule.setpoint_enforcement,
                frequency_seconds: Number(event.target.value),
              },
            },
          }))}
          size="small"
          value={data?.local_schedule?.setpoint_enforcement?.frequency_seconds?.toString() || ''}
          validationRegEx='positiveIntegers'
        />
      </CardRow>
      <CardRow alignItems="center">
        <Typography
          sx={{ width: theme.fgb.siteFleetConfig.sizing.typographyWidth }}
          variant="body2"
        >
          {SClabels.server.main}
        </Typography>
        <Box sx={switchBoxSx}>
          <Switch
            color="primary"
            label={SClabels.server.switch}
            labelPlacement="right"
            onChange={(event) => setConfigEdits((prevState: any) => ({
              ...prevState,
              web_sockets: {
                ...prevState.web_sockets,
                server: {
                  ...prevState.web_sockets?.server,
                  enabled: !prevState.web_sockets?.server?.enabled || event,
                },
              },
            }))}
            value={data?.web_sockets?.server?.enabled || false}
          />
        </Box>
        <NumericInput
          label={SClabels.server.textField}
          endComponentAdorment={ <></> }
          onChange={(event) => setConfigEdits((prevState: any) => ({
            ...prevState,
            web_sockets: {
              ...prevState.web_sockets,
              server: {
                ...prevState.web_sockets?.server,
                port: (Number(event.target.value) > 0
                  ? Number(event.target.value) : 1),
              },
            },
          }))}
          size="small"
          value={data?.web_sockets?.server?.port.toString() || ''}
          validationRegEx='positiveIntegers'
        />
      </CardRow>
      <CardRow alignItems="start">
        <Typography
          sx={{ width: theme.fgb.siteFleetConfig.sizing.typographyWidth }}
          variant="body2"
        >
          {labels.SCADA.main}
        </Typography>
        <Box sx={switchBoxSx}>
          <Switch
            color="primary"
            label={labels.SCADA.switch}
            labelPlacement="right"
            onChange={() => setShowSCADA(!showSCADA)}
          />
        </Box>
        {showSCADA && (
        <SCADASettings setConfigEdits={setConfigEdits} settings={data?.scada} />
        )}
      </CardRow>
    </Box>
  );
};

export default SiteControllerConfig;
