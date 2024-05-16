/* eslint-disable */
// TODO: fix lint
import {
  ThemeType,
  Switch,
  CardRow,
  TextField,
  Typography,
  IconButton,
  NumericInput,
} from '@flexgen/storybook';
import { Box, Collapse } from '@mui/material';
import React, { useEffect, useState } from 'react';
import { Configuration } from 'shared/types/dtos/scheduler.dto';
import FleetSites from 'src/pages/Scheduler/SchedulerConfiguration/FleetManagerConfig/SiteList';
import {
  fleetLSBoxSx,
  fleetManagerLabels as FMlabels,
  collapseSx,
  siteFleetConfigLabels as labels,
  FleetSiteActions,
  IPregex,
  nameRegex,
} from 'src/pages/Scheduler/SchedulerConfiguration/Helpers';
import SCADASettings from 'src/pages/Scheduler/SchedulerConfiguration/SCADASettings';
import { useTheme } from 'styled-components';
import { v4 as uuid } from 'uuid';

interface FleetManagerConfigProps {
  // configEdits
  data?: Configuration | null;
  setConfigEdits: React.Dispatch<React.SetStateAction<Configuration | null | undefined>>;
  setSaveDisabled: any;
}

const FleetManagerConfig: React.FC<FleetManagerConfigProps> = ({
  data,
  setConfigEdits,
  setSaveDisabled,
}: FleetManagerConfigProps) => {
  const theme = useTheme() as ThemeType;
  const localScheduleSx = fleetLSBoxSx(theme);
  const [open, setOpen] = useState<boolean>(false);
  const [showSCADA, setShowSCADA] = useState(false);
  const [index, setIndex] = useState<any>(null);
  const [nameFieldMissing, setNameFieldMissing] = useState<boolean>(false);
  const [siteError, setSiteError] = useState<boolean>(false);

  /** Checks for local schedule */
  useEffect(() => {
    if (!data?.local_schedule) return setNameFieldMissing(true);
    const path = data.local_schedule;
    const missing = !path.name || nameRegex.test(path.name);
    setNameFieldMissing(missing);
  }, [data]);

  const checkSiteError = () => {
    if (!data?.web_sockets?.clients || data?.web_sockets?.clients.length === 0) return;
    const sites = data.web_sockets.clients;
    const nameSet = new Set<string>();
    for (const obj of sites) {
      const name = obj.name.toLowerCase();
      if (nameSet.has(name) || !IPregex.test(obj.ip) || nameRegex.test(name)) {
        return setSiteError(true);
      }
      nameSet.add(name);
    }
    return setSiteError(false);
  };

  useEffect(() => {
    checkSiteError();
    if (
      !data?.web_sockets?.clients ||
      data?.web_sockets?.clients.length === 0 ||
      !data?.web_sockets?.clients[index]
    )
      setIndex(null);
  }, [data, index, setSiteError]);

  useEffect(() => {
    const missingLSName =
      (data?.local_schedule !== undefined && !data.local_schedule.name) ||
      (data?.local_schedule !== undefined &&
        data?.local_schedule.name &&
        nameRegex.test(data?.local_schedule?.name));
    setSaveDisabled(siteError || missingLSName);
  }, [siteError, data]);

  const updateSiteData = (action: FleetSiteActions) => {
    if (!data?.web_sockets?.clients) return;
    const arrayCopy = [...data.web_sockets.clients];
    if (index === undefined) return;
    if (action === 'delete') {
      setSiteError(false);
      arrayCopy.splice(index, 1);
      setIndex(null);
    }
    setConfigEdits((prevState: any) => ({
      ...prevState,
      web_sockets: {
        clients: arrayCopy,
      },
    }));
    checkSiteError();
  };

  const addSite = () => {
    const newSite = {
      id: uuid(),
      name: 'New Site',
      ip: '',
      port: 9000,
    };
    const sites = data?.web_sockets?.clients;
    if (sites) {
      setConfigEdits((prevState: any) => ({
        ...prevState,
        web_sockets: {
          clients: [...prevState.web_sockets.clients, newSite],
        },
      }));
      setIndex(sites.length);
    } else {
      setConfigEdits((prevState: any) => ({
        ...prevState,
        web_sockets: {
          clients: [newSite],
        },
      }));
      setIndex(0);
    }
    checkSiteError();
  };

  return (
    <Box sx={{ display: 'flex', flexDirection: 'row', flex: 1, paddingBottom: '12px' }}>
      <Collapse collapsedSize='48px' in={open} orientation='horizontal' sx={collapseSx(theme)}>
        <Box sx={localScheduleSx}>
          <div style={{ marginLeft: '4px' }}>
            <IconButton icon='Settings' onClick={() => setOpen(!open)} />
          </div>
          <Box>
            <CardRow alignItems='center' styleOverrides={{ paddingTop: '8px' }}>
              <Typography text={FMlabels.scadaSettings} variant='bodyLBold' />
            </CardRow>
            <CardRow alignItems='center'>
              <SCADASettings setConfigEdits={setConfigEdits} settings={data?.scada} />
            </CardRow>
            <CardRow alignItems='center'>
              <Typography text={FMlabels.settings} variant='bodyLBold' />
            </CardRow>
            <CardRow styleOverrides={{ width: '100%' }}>
              <TextField
                fullWidth
                helperText={nameFieldMissing ? FMlabels.localSchedule.missingNameField : undefined}
                label={FMlabels.localSchedule.name}
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
            </CardRow>
            <CardRow>
              <Switch
                color='primary'
                disabled={nameFieldMissing}
                label={FMlabels.localSchedule.clothedSetpoints}
                labelPlacement='right'
                onChange={(event) =>
                  setConfigEdits((prevState: any) => ({
                    ...prevState,
                    local_schedule: {
                      ...prevState.local_schedule,
                      clothed_setpoints: !prevState.local_schedule?.clothed_setpoints || event,
                    },
                  }))
                }
                value={data?.local_schedule?.clothed_setpoints || false}
              />
            </CardRow>
            <CardRow>
              <Switch
                color='primary'
                disabled={nameFieldMissing}
                label={FMlabels.localSchedule.setPointEnforcement.switch}
                labelPlacement='right'
                onChange={(event) =>
                  setConfigEdits((prevState: any) => ({
                    ...prevState,
                    local_schedule: {
                      ...prevState.local_schedule,
                      setpoint_enforcement: {
                        ...prevState.local_schedule?.setpoint_enforcement,
                        enabled: !prevState.local_schedule?.setpoint_enforcement?.enabled || event,
                      },
                    },
                  }))
                }
                value={data?.local_schedule?.setpoint_enforcement?.enabled || false}
              />
            </CardRow>
            <CardRow>
              <NumericInput
                fullWidth
                disabled={nameFieldMissing}
                endComponentAdorment={<></>}
                label={FMlabels.localSchedule.setPointEnforcement.textField}
                onChange={(event) =>
                  setConfigEdits((prevState: any) => ({
                    ...prevState,
                    local_schedule: {
                      ...prevState.local_schedule,
                      setpoint_enforcement: {
                        ...prevState.local_schedule?.setpoint_enforcement,
                        frequency_seconds: Number(event.target.value),
                      },
                    },
                  }))
                }
                value={
                  data?.local_schedule?.setpoint_enforcement?.frequency_seconds?.toString() || ''
                }
                validationRegEx='positiveIntegers'
              />
            </CardRow>
          </Box>
        </Box>
      </Collapse>
      <FleetSites
        addSite={addSite}
        index={index}
        setConfigEdits={setConfigEdits}
        setIndex={setIndex}
        sites={data?.web_sockets?.clients}
        updateSiteData={updateSiteData}
      />
    </Box>
  );
};

export default FleetManagerConfig;
