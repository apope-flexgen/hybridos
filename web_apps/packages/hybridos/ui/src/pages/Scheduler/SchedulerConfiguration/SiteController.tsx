/* eslint-disable */
// TODO: fix lint
import { ThemeType, Switch, CardRow, TextField, NumericInput } from '@flexgen/storybook';
import { Box, Typography } from '@mui/material';
import React, { useEffect } from 'react';
import { Configuration } from 'shared/types/dtos/scheduler.dto';
import { useTheme } from 'styled-components';
import {
  siteFleetConfigLabels as labels,
  siteControllerLabels as SClabels,
  nameRegex,
} from './Helpers';
import SCADASettings from './SCADASettings';

interface SiteControllerConfigProps {
  data?: Configuration | null;
  setConfigEdits: React.Dispatch<React.SetStateAction<Configuration | null | undefined>>;
  setSaveDisabled: any;
}

const SiteControllerConfig: React.FC<SiteControllerConfigProps> = ({
  data,
  setConfigEdits,
  setSaveDisabled,
}: SiteControllerConfigProps) => {
  const theme = useTheme() as ThemeType;
  const [showSCADA, setShowSCADA] = React.useState(false);
  const [nameError, setNameError] = React.useState<boolean>(false);

  useEffect(() => {
    const error =
      !data?.local_schedule?.name ||
      nameRegex.test(data.local_schedule.name) ||
      data.local_schedule.name === '';
    setNameError(error);
    setSaveDisabled(error);
  }, [data]);

  return (
    <Box>
      <CardRow alignItems='center'>
        <Typography
          sx={{
            marginTop: theme.fgb.siteFleetConfig.sizing.typographyMarginTop,
          }}
          variant='h2'
        >
          {SClabels.settings}
        </Typography>
      </CardRow>
      <CardRow alignItems='center'>
        <Typography
          sx={{ width: theme.fgb.siteFleetConfig.sizing.typographyWidth }}
          variant='body2'
        >
          {SClabels.siteName}
        </Typography>
        <Box sx={{ width: '500px' }}>
          <TextField
            color={nameError ? 'error' : 'primary'}
            fullWidth
            helperText={nameError ? SClabels.nameError : undefined}
            label={SClabels.siteName}
            onChange={(event) =>
              setConfigEdits((prevState: any) => ({
                ...prevState,
                local_schedule: {
                  ...prevState.local_schedule,
                  name: event.target.value,
                },
              }))
            }
            size='small'
            value={data?.local_schedule?.name || ''}
          />
        </Box>
      </CardRow>
      <CardRow alignItems='center'>
        <Typography
          sx={{ width: theme.fgb.siteFleetConfig.sizing.typographyWidth }}
          variant='body2'
        >
          {SClabels.clothedSetpoints.main}
        </Typography>
        <Switch
          autoLayout
          color='primary'
          label={SClabels.clothedSetpoints.switch}
          labelPlacement='right'
          onChange={(event) =>
            setConfigEdits((prevState: any) => ({
              ...prevState,
              local_schedule: {
                ...prevState.local_schedule,
                clothed_setpoints: !prevState.local_schedule.clothed_setpoints || event,
              },
            }))
          }
          value={data?.local_schedule?.clothed_setpoints || false}
        />
      </CardRow>
      <CardRow alignItems='center'>
        <Typography
          sx={{ width: theme.fgb.siteFleetConfig.sizing.typographyWidth }}
          variant='body2'
        >
          {SClabels.setpointEnforcement.main}
        </Typography>
        <Box sx={{ width: '240px' }}>
          <Switch
            autoLayout
            color='primary'
            label={SClabels.setpointEnforcement.switch}
            labelPlacement='right'
            onChange={(event) =>
              setConfigEdits((prevState: any) => ({
                ...prevState,
                local_schedule: {
                  ...prevState.local_schedule,
                  setpoint_enforcement: {
                    ...prevState.local_schedule.setpoint_enforcement,
                    enabled: !prevState.local_schedule?.setpoint_enforcement?.enabled || event,
                  },
                },
              }))
            }
            value={data?.local_schedule?.setpoint_enforcement?.enabled || false}
          />
        </Box>
        <NumericInput
          label={SClabels.setpointEnforcement.textField}
          endComponentAdorment={<></>}
          onChange={(event) =>
            setConfigEdits((prevState: any) => ({
              ...prevState,
              local_schedule: {
                ...prevState.local_schedule,
                setpoint_enforcement: {
                  ...prevState.local_schedule.setpoint_enforcement,
                  frequency_seconds: Number(event.target.value),
                },
              },
            }))
          }
          size='small'
          value={data?.local_schedule?.setpoint_enforcement?.frequency_seconds?.toString() || ''}
          validationRegEx='positiveIntegers'
        />
      </CardRow>
      <CardRow alignItems='center'>
        <Typography
          sx={{ width: theme.fgb.siteFleetConfig.sizing.typographyWidth }}
          variant='body2'
        >
          {SClabels.server.main}
        </Typography>
        <Box sx={{ width: '240px' }}>
          <Switch
            autoLayout
            color='primary'
            label={SClabels.server.switch}
            labelPlacement='right'
            onChange={(event) =>
              setConfigEdits((prevState: any) => ({
                ...prevState,
                web_sockets: {
                  ...prevState.web_sockets,
                  server: {
                    ...prevState.web_sockets?.server,
                    enabled: !prevState.web_sockets?.server?.enabled || event,
                  },
                },
              }))
            }
            value={data?.web_sockets?.server?.enabled || false}
          />
        </Box>
        <NumericInput
          label={SClabels.server.textField}
          endComponentAdorment={<></>}
          onChange={(event) =>
            setConfigEdits((prevState: any) => ({
              ...prevState,
              web_sockets: {
                ...prevState.web_sockets,
                server: {
                  ...prevState.web_sockets?.server,
                  port: Number(event.target.value) > 0 ? Number(event.target.value) : 1,
                },
              },
            }))
          }
          size='small'
          value={data?.web_sockets?.server?.port.toString() || ''}
          validationRegEx='positiveIntegers'
        />
      </CardRow>
      <CardRow alignItems='start'>
        <Typography
          sx={{ width: theme.fgb.siteFleetConfig.sizing.typographyWidth }}
          variant='body2'
        >
          {labels.SCADA.main}
        </Typography>
        <Box sx={{ width: '240px' }}>
          <Switch
            autoLayout
            color='primary'
            label={labels.SCADA.switch}
            labelPlacement='right'
            onChange={() => setShowSCADA(!showSCADA)}
          />
        </Box>
        {showSCADA && <SCADASettings setConfigEdits={setConfigEdits} settings={data?.scada} />}
      </CardRow>
    </Box>
  );
};

export default SiteControllerConfig;
