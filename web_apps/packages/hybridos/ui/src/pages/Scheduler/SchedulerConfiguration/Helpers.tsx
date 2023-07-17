/* eslint-disable max-lines */
import { ThemeType } from '@flexgen/storybook';
import { createTheme, Theme } from '@mui/material/styles';
import { Configuration } from 'shared/types/dtos/scheduler.dto';
import { v4 as uuid } from 'uuid';

export type FleetSiteActions = 'delete' | 'cancel';

export type SchedulerTypes = 'SC' | 'FM';

export function createMuiTheme(theme: ThemeType): Theme {
  return createTheme({
    typography: {
      h1: {
        fontFamily: theme.fgb.schedulerConfig.fonts.siteConfigName.fontFamily,
        fontWeight: theme.fgb.schedulerConfig.fonts.siteConfigName.fontWeight,
        fontSize: `${theme.fgb.schedulerConfig.fonts.siteConfigName.fontSize}px`,
        color: theme.fgc.scheduler.fontColor,
      },
      body1: {
        fontFamily: theme.fgb.schedulerConfig.fonts.siteConfigHelper.fontFamily,
        fontWeight: theme.fgb.schedulerConfig.fonts.siteConfigHelper.fontWeight,
        fontSize: `${theme.fgb.schedulerConfig.fonts.siteConfigHelper.fontSize}px`,
        color: theme.fgc.scheduler.fontColor,
      },
      body2: {
        fontFamily: theme.fgb.siteFleetConfig.controlsInfo.fontFamily,
        fontWeight: theme.fgb.siteFleetConfig.controlsInfo.fontWeight,
        fontSize: `${theme.fgb.siteFleetConfig.controlsInfo.fontSize}px`,
        color: theme.fgc.scheduler.fontColor,
      },
      h2: {
        fontFamily: theme.fgb.siteFleetConfig.controlsTitle.fontFamily,
        fontWeight: theme.fgb.siteFleetConfig.controlsTitle.fontWeight,
        fontSize: `${theme.fgb.siteFleetConfig.controlsTitle.fontSize}px`,
        color: theme.fgc.scheduler.fontColor,
      },
      h3: {
        fontFamily: theme.fgb.schedulerConfig.fonts.modes.fontFamily,
        fontWeight: theme.fgb.schedulerConfig.fonts.modes.fontWeight,
        fontSize: `${theme.fgb.schedulerConfig.fonts.modes.fontSize}px`,
        color: theme.fgc.scheduler.fontColor,
      },
    },
  });
}

export const schedulerConfigSx = (theme: ThemeType) => ({
  height: '100%',
  minHeight: '500px',
  paddingTop: theme.fgb.siteFleetConfig.sizing.padding,
  width: '100%',
});

export const headerSx = (theme: ThemeType) => ({
  display: 'flex',
  padding: theme.fgb.siteFleetConfig.sizing.padding,
  paddingTop: 0,
  alignItems: 'center',
  boxShadow: `0px 15px 10px -20px ${theme.fgd.brand.dark}`,
});

export const buttonBoxSx = (theme: ThemeType) => ({
  marginLeft: 'auto',
  display: 'flex',
  gap: theme.fgb.siteFleetConfig.sizing.padding,
});

export const fleetSiteListBoxSx = (theme: ThemeType) => ({
  minWidth: '18%',
  minHeight: '500px',
  padding: theme.fgb.schedulerConfig.sizing.padding,
  paddingBottom: '12px',
  display: 'flex',
  backgroundColor: `${theme.fgd.primary.main}20`,
  flexDirection: 'column',
  gap: '10px',
});

export const collapseSx = (theme: ThemeType) => ({
  boxShadow: `15px 0px 10px -20px ${theme.fgd.brand.dark}`,
  maxWidth: '328px',
});

export const fleetLSBoxSx = (theme: ThemeType) => ({
  marginTop: theme.fgb.schedulerConfig.sizing.padding,
  display: 'flex',
  minHeight: '500px',
  height: '100%',
  flexDirection: 'row',
});

export const SCADASettingsSx = (theme: ThemeType) => ({
  box: {
    display: 'flex',
    flexDirection: 'column',
    gap: theme.fgb.siteFleetConfig.sizing.padding,
  },
  boxRow: {
    display: 'flex',
    gap: theme.fgb.siteFleetConfig.sizing.padding,
    width: '250px',
  },
});

export const siteFleetConfigLabels = {
  notifications: {
    success: 'Configuration successfully saved',
  },
  pageDescription: {
    sc: 'Site Controller Configuration',
    fm: 'Fleet Manager Configuration',
    unconfigured: 'Initial Scheduler Setup',
  },
  buttons: {
    save: 'Save',
    cancel: 'Cancel',
  },
  helperText: {
    name: 'A name must be provided. No duplicate names allowed.',
  },
  SCADA: {
    main: 'SCADA',
    switch: 'View SCADA Settings',
    stageSize: 'Stage Size',
    maxEvents: 'Max Events',
    ints: 'Ints',
    floats: 'Floats',
    booleans: 'Booleans',
    strings: 'Strings',
  },
};

export const siteControllerLabels = {
  settings: 'Settings',
  siteName: 'Site Name',
  clothedSetpoints: {
    main: 'Use Clothed Setpoints',
    switch: 'Enable',
  },
  setpointEnforcement: {
    main: 'Setpoint Enforcement',
    switch: 'Enable',
    textField: 'Frequency (seconds)',
  },
  server: {
    main: 'Server',
    switch: 'Enable',
    textField: 'Port',
  },
  serverTextLabel: 'Port',
  nameError: 'This field is required',
};

export const fleetManagerLabels = {
  sites: 'Sites',
  addSite: 'Add Site',
  settings: 'Fleet Settings (optional)',
  scadaSettings: 'SCADA Settings',
  noItem: 'Select a Site to View Site Configuration',
  duplicateName: 'Site Names must be unique',
  missingName: 'Please enter a site name',
  localSchedule: {
    missingNameField: 'To edit Fleet Settings, please provide a name',
    name: 'Name',
    clothedSetpoints: 'Use Clothed Setpoints',
    setPointEnforcement: {
      switch: 'Enable Setpoint Enforcement',
      textField: 'Frequency (seconds)',
    },
    cancel: 'Cancel',
    save: 'Save Fleet Settings',
  },
  clientSettings: {
    title: 'Settings',
    cancel: 'Cancel',
    save: 'Save',
    delete: 'Delete',
    id: 'ID',
    name: 'Name',
    ip: 'IP',
    invalidIPHelper: 'Please enter a valid IP address',
    port: 'Port',
  },
};

// eslint-disable-next-line max-len
export const IPregex = /^(?!0)(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/;

export const nameRegex = /^\s*$/;

export const initialSCConfig: Configuration = {
  scheduler_type: 'SC',
  local_schedule: {
    id: uuid(),
    name: '',
    clothed_setpoints: false,
    setpoint_enforcement: {
      enabled: false,
      frequency_seconds: 360000,
    },
  },
  web_sockets: {
    server: {
      enabled: false,
      port: 1,
    },
  },
  scada: {
    stage_size: 1,
    max_num_events: 100,
    num_floats: 5,
    num_ints: 5,
    num_bools: 5,
    num_strings: 5,
  },
};

export const initialFMConfig: Configuration = {
  scheduler_type: 'FM',
  web_sockets: {
    clients: [],
  },
  scada: {
    stage_size: 1,
    max_num_events: 100,
    num_floats: 5,
    num_ints: 5,
    num_bools: 5,
    num_strings: 5,
  },
};
