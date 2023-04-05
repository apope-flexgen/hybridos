/* eslint-disable max-lines */
import {
  ThemeType, Switch, CardRow, TextField, Typography, IconButton, NumericInput
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
} from 'src/pages/Scheduler/SchedulerConfiguration/Helpers';
import SCADASettings from 'src/pages/Scheduler/SchedulerConfiguration/SCADASettings';
import { useTheme } from 'styled-components';
import { v4 as uuid } from 'uuid';

interface FleetManagerConfigProps {
  // configEdits
  data?: Configuration | null
  setConfigEdits: React.Dispatch<React.SetStateAction<Configuration | null | undefined>>
  setSaveDisabled: any
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

  /** Checks for local schedule */
  useEffect(() => {
    if (!data?.local_schedule) return;
    const path = data.local_schedule;
    if ((!path.name || path.name === '') && (path.clothed_setpoints || path.setpoint_enforcement)) {
      setSaveDisabled(true);
      setNameFieldMissing(true);
    } else {
      setSaveDisabled(false);
      setNameFieldMissing(false);
    }
  }, [data, setSaveDisabled]);

  /** Checks for duplicate names / reset index */
  useEffect(() => {
    if (!data?.web_sockets?.clients
            || data?.web_sockets?.clients.length === 0
            || !data?.web_sockets?.clients[index]) return setIndex(null);

    const sites = data.web_sockets.clients;
    const nameSet = new Set<string>();
    // TODO
    // eslint-disable-next-line no-restricted-syntax
    for (const obj of sites) {
      if (nameSet.has(obj.name) || !IPregex.test(obj.ip)) {
        return setSaveDisabled(true);
      }
      nameSet.add(obj.name);
    }
    return setSaveDisabled(false);
  }, [data, index, setSaveDisabled]);

  const updateSiteData = (action: FleetSiteActions) => {
    if (!data?.web_sockets?.clients) return;
    const arrayCopy = [...data.web_sockets.clients];
    if (index === undefined) return;
    if (action === 'delete') {
      arrayCopy.splice(index, 1);
      setIndex(null);
    }
    setConfigEdits((prevState: any) => ({
      ...prevState,
      web_sockets: {
        clients: arrayCopy,
      },
    }));
  };

  const addSite = () => {
    const newSite = {
      id: uuid(), name: 'New Site', ip: '', port: 1,
    };
    if (data?.web_sockets?.clients) {
      setConfigEdits((prevState: any) => ({
        ...prevState,
        web_sockets: {
          clients: [
            ...prevState.web_sockets.clients,
            newSite,
          ],
        },
      }));
    } else {
      setConfigEdits((prevState: any) => ({
        ...prevState,
        webS_sockets: {
          clients: [
            newSite,
          ],
        },
      }));
    }
  };

  return (
    <Box sx={{ display: 'flex', flexDirection: 'row', flex: 1 }}>
      <Collapse collapsedSize="48px" in={open} orientation="horizontal" sx={collapseSx(theme)}>
        <Box sx={localScheduleSx}>
          <div style={{ marginLeft: '4px' }}>
            <IconButton icon="Settings" onClick={() => setOpen(!open)} />
          </div>
          <Box>
            <CardRow alignItems="center">
              <Typography text={FMlabels.settings} variant="bodyLBold" />
            </CardRow>
            <CardRow>
              <TextField
                helperText={nameFieldMissing ? FMlabels.localSchedule.missingNameField : undefined}
                label={FMlabels.localSchedule.name}
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
            </CardRow>
            <CardRow>
              <Switch
                color="primary"
                label={FMlabels.localSchedule.clothedSetpoints}
                labelPlacement="right"
                onChange={(event) => setConfigEdits((prevState: any) => ({
                  ...prevState,
                  local_schedule: {
                    ...prevState.local_schedule,
                    clothed_setpoints: !prevState.local_schedule?.clothed_setpoints || event,
                  },
                }))}
                value={data?.local_schedule?.clothed_setpoints || false}
              />
            </CardRow>
            <CardRow>
              <Switch
                color="primary"
                label={FMlabels.localSchedule.setPointEnforcement.switch}
                labelPlacement="right"
                onChange={(event) => setConfigEdits((prevState: any) => ({
                  ...prevState,
                  local_schedule: {
                    ...prevState.local_schedule,
                    setpoint_enforcement: {
                      ...prevState.local_schedule?.setpoint_enforcement,
                      enabled: !prevState.local_schedule?.setpoint_enforcement?.enabled || event,
                    },
                  },
                }))}
                value={data?.local_schedule?.setpoint_enforcement?.enabled || false}
              />
            </CardRow>
            <CardRow>
              <NumericInput
                fullWidth
                endComponentAdorment={ <></> }
                label={FMlabels.localSchedule.setPointEnforcement.textField}
                onChange={(event) => setConfigEdits((prevState: any) => ({
                  ...prevState,
                  local_schedule: {
                    ...prevState.local_schedule,
                    setpoint_enforcement: {
                      ...prevState.local_schedule?.setpoint_enforcement,
                      frequency_seconds: Number(event.target.value),
                    },
                  },
                }))}
                value={data?.local_schedule?.setpoint_enforcement?.frequency_seconds?.toString() || ''}
                validationRegEx='positiveIntegers'
              />
            </CardRow>
            <CardRow>
              <Switch
                color="primary"
                label={labels.SCADA.switch}
                labelPlacement="right"
                onChange={() => setShowSCADA(!showSCADA)}
                value={false}
              />
            </CardRow>
            <CardRow alignItems="center">
              {showSCADA
              && <SCADASettings setConfigEdits={setConfigEdits} settings={data?.scada} />}
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
