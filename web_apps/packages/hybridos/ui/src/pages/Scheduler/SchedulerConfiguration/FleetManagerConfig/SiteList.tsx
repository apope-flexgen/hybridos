/* eslint-disable max-lines */
import {
  MuiButton, Typography, TextField, ThemeType,
} from '@flexgen/storybook';
import Grid from '@mui/material/Grid';
import { Box } from '@mui/system';
import React, { useEffect, useState } from 'react';
import {
  FleetSiteActions, fleetSiteListBoxSx, buttonBoxSx, IPregex,
  fleetManagerLabels as FMlabels, noSelectedSiteSx,
} from 'src/pages/Scheduler/SchedulerConfiguration/Helpers';
import { useTheme } from 'styled-components';

interface FleetSitesProps {
  addSite: () => void,
  index: number | null,
  sites: any,
  setIndex: (newValue: any) => void,
  setConfigEdits: any,
  updateSiteData: (action: FleetSiteActions) => void,
}

const FleetSites: React.FC<FleetSitesProps> = ({
  addSite,
  sites,
  updateSiteData,
  setConfigEdits,
  index,
  setIndex,
}: FleetSitesProps) => {
  const theme = useTheme() as ThemeType;
  const boxStyles = fleetSiteListBoxSx(theme);
  const buttonSx = buttonBoxSx(theme);
  const [nameError, setNameError] = useState<boolean>(false);
  const [validIP, setValidIP] = useState<boolean>(true);

  useEffect(() => {
    if (sites == null || index === null || !sites[index]) return;
    setValidIP(IPregex.test(sites[index].ip));
    setNameError(!!(sites.find((obj: any) => obj.name === sites[index].name
    && obj.id !== sites[index].id)));
  }, [sites, index]);

  const updateField = (newValue: string | number, property: string) => {
    const arrayCopy = [...sites];
    if (index === null) return;
    arrayCopy[index] = {
      ...arrayCopy[index],
      [property]: newValue,
    };
    setConfigEdits((prevState: any) => ({
      ...prevState,
      web_sockets: {
        clients: arrayCopy,
      },
    }));
  };

  return (
    <>
      <Box sx={boxStyles}>
        <Typography text={FMlabels.sites} variant="bodyLBold" />
        {sites
                    && sites.map((site: any) => (
                      <MuiButton
                        color="primary"
                        fullWidth
                        label={site.name}
                        onClick={() => setIndex(sites.findIndex((obj: any) => obj.id === site.id))}
                        size="large"
                        variant={(index !== null && sites[index] && sites[index].id === site.id) ? 'contained' : 'outlined'}
                      />
                    ))}
        <div>
          <MuiButton
            fullWidth
            label={FMlabels.addSite}
            onClick={addSite}
            size="large"
            startIcon="Add"
            variant="outlined"
          />
        </div>
      </Box>
      <Box sx={{ flexGrow: 1 }}>
        {(sites && index !== null && sites[index])
          ? (
            <Box sx={{ flex: 1, padding: '16px', maxWidth: '550px' }}>
              <Grid container spacing={2}>
                <Grid item xs={10}>
                  <Typography
                    text={`${sites[index].name} ${FMlabels.clientSettings.title}`}
                    variant="headingS"
                  />
                </Grid>
                <Grid item xs={4}>
                  <Typography text={FMlabels.clientSettings.name} variant="bodyL" />
                </Grid>
                <Grid item xs={6}>
                  <TextField
                    color={nameError ? 'error' : 'primary'}
                    fullWidth
                    helperText={nameError ? FMlabels.duplicateName : undefined}
                    label={FMlabels.clientSettings.name}
                    onChange={(event) => updateField(event.target.value, 'name')}
                    value={sites[index].name}
                  />
                </Grid>
                <Grid item xs={4}>
                  <Typography text={FMlabels.clientSettings.ip} variant="bodyL" />
                </Grid>
                <Grid item xs={6}>
                  <TextField
                    helperText={!validIP ? FMlabels.clientSettings.invalidIPHelper : undefined}
                    fullWidth
                    label={FMlabels.clientSettings.ip}
                    onChange={(event) => {
                      updateField(String(event.target.value), 'ip');
                      setValidIP(IPregex.test(event.target.value));
                    }}
                    value={sites[index].ip}
                  />
                </Grid>
                <Grid item xs={4}>
                  <Typography text={FMlabels.clientSettings.port} variant="bodyL" />
                </Grid>
                <Grid item xs={6}>
                  <TextField
                    inputProps={{ min: 1 }}
                    fullWidth
                    label={FMlabels.clientSettings.port}
                    onChange={(event) => updateField(
                      (Number(event.target.value) > 0
                        ? Number(event.target.value) : 1),
                      'port',
                    )}
                    value={sites[index].port}
                    type="number"
                  />
                </Grid>
                <Grid item xs={4} />
                <Grid item xs={6}>
                  <Box style={buttonSx}>
                    <MuiButton
                      color="error"
                      label={FMlabels.clientSettings.delete}
                      onClick={() => updateSiteData('delete')}
                      size="small"
                      variant="outlined"
                    />
                  </Box>
                </Grid>
              </Grid>
            </Box>
          )
          : (
            <Box sx={noSelectedSiteSx(theme)}>
              <Typography text={FMlabels.noItem} variant="headingM" />
            </Box>
          )}
      </Box>
    </>
  );
};

export default FleetSites;
