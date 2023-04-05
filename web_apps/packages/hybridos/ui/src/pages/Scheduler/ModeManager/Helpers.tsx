// TODO: fix lint
/* eslint-disable consistent-return, max-lines */
import { ThemeType } from '@flexgen/storybook';
import { createTheme, Theme } from '@mui/material/styles';
import { v4 as uuidv4 } from 'uuid';
import { ModeBody } from './Types';

export function createMuiTheme(theme: ThemeType): Theme {
  return createTheme({
    components: {
      MuiPaper: {
        styleOverrides: {
          root: {
            boxShadow: `${theme.fgb.schedulerConfig.shadow.shadow_01} ${theme.fgc.expandingTable.shadow}, 
                        ${theme.fgb.schedulerConfig.shadow.shadow_02} ${theme.fgc.expandingTable.shadow}, 
                        ${theme.fgb.schedulerConfig.shadow.shadow_03} ${theme.fgc.expandingTable.shadow}`,
          },
        },
      },
      MuiTable: {
        styleOverrides: {
          root: {
            backgroundColor: theme.fgc.expandingTable.background,
          },
        },
      },
    },
    typography: {
      h1: {
        fontFamily: theme.fgb.schedulerConfig.fonts.siteConfigName.fontFamily,
        fontWeight: theme.fgb.schedulerConfig.fonts.siteConfigName.fontWeight,
        fontSize: `${theme.fgb.schedulerConfig.fonts.siteConfigName.fontSize}px`,
        color: theme.fgc.scheduler.fontColor,
      },
      h2: {
        fontFamily: theme.fgb.schedulerConfig.fonts.siteConfigHelper.fontFamily,
        fontWeight: theme.fgb.schedulerConfig.fonts.siteConfigHelper.fontWeight,
        fontSize: `${theme.fgb.schedulerConfig.fonts.siteConfigHelper.fontSize}px`,
        color: theme.fgc.scheduler.fontColor,
      },
      h3: {
        fontFamily: theme.fgb.schedulerConfig.fonts.modes.fontFamily,
        fontWeight: theme.fgb.schedulerConfig.fonts.modes.fontWeight,
        fontSize: `${theme.fgb.schedulerConfig.fonts.modes.fontSize}px`,
        color: theme.fgc.scheduler.fontColor,
      },
      body1: {
        fontFamily: theme.fgb.schedulerConfig.fonts.modeProps.fontFamily,
        fontWeight: theme.fgb.schedulerConfig.fonts.modeProps.fontWeight,
        fontSize: `${theme.fgb.schedulerConfig.fonts.modeProps.fontSize}px`,
        color: theme.fgc.scheduler.fontColor,
      },
    },
  });
}

export const modeListBoxSx = (theme: ThemeType) => ({
  width: '23%',
  height: '100%',
  padding: theme.fgb.schedulerConfig.sizing.padding,
  display: 'flex',
  flexDirection: 'column',
  gap: '10px',
});

export const headerSx = (theme: ThemeType) => ({
  backgroundColor: theme.fgc.expandingTable.header,
});

export const icons = [
  'BatteryVert',
  'Bolt',
  'OfflineBolt',
  'Timelapse',
  'Sun',
  'Moon',
  'DateRange',
  'Build',
  'BatteryCharging',
  'ArrowUp',
  'ArrowDown',
];

export const blankMode: ModeBody = {
  name: 'New Mode',
  color_code: 'gray',
  icon: 'ArrowUp',
  variables: [],
  constants: [],
};

export const blankSetpoint = {
  id: '',
  name: '',
  type: '',
  unit: '',
  uri: '',
  value: undefined,
};

/** This following function should probably be done on the backend. */
export function randomID(): string {
  const uuid = uuidv4();
  const id = uuid.toString().replaceAll('-', '');
  return id;
}

export function newType(type?: any): object {
  const data = { ...blankSetpoint };
  const setpointType = type.substring(0, type.length - 1);
  data.id = randomID();
  data.name = `New ${setpointType}`;
  return data;
}

export function newMode(): [string, ModeBody] {
  const mode = { ...blankMode };
  const id = randomID();
  return [id, mode];
}

export const modeInfoSizing = {
  textWidth: '150px',
  selectorWidth: '500px',
  setpointMargin: '10px',
};

export const schedulerConfigLabels = {
  siteInformation: {
    sc: 'Site Controller Mode Manager',
    fm: 'Fleet Manager Mode Manager',
  },
  modeList: {
    addButton: {
      label: 'Add New Mode',
    },
  },
  modeInfo: {
    title: {
      selectedModeId: 'Configuration Settings',
      noselectedModeId: 'Select Mode to View Configuration Settings',
    },
    buttons: {
      cancel: 'Cancel',
      save: 'Save',
      delete: 'delete',
    },
    tooltip: {
      defaultMode: 'A default mode is required. It cannot be deleted, and the name cannot be edited.',
      addModeButton: 'New Mode must have a new unique name before adding more modes.',
      nameFieldHelperText: 'Mode names must be unique',
      newModeNameHelperText: 'Provide a unique name for the new mode',
    },
    setpointRow: {
      name: 'Name',
      type: 'Type',
      unit: 'Unit',
      URI: 'URI',
      value: 'Value',
      defaultValueHelper: 'Type of value entered must match Type chosen above.',
      delete: 'Delete Variable',
    },
  },
};

export const handleIconColor = (
  icon: string,
  selectedModeValues: ModeBody | null | undefined,
) => {
  if (!selectedModeValues) return;
  if (selectedModeValues.icon === icon) return 'primary';
  return undefined;
};

export const updateModeFields = (
  field: 'name' | 'color' | 'icon',
  newValue: string | undefined,
  setselectedModeValues: React.Dispatch<React.SetStateAction<any>>,
  event?: any,
) => {
  switch (field) {
    case 'name':
      setselectedModeValues((prevState: ModeBody) => ({
        ...prevState,
        name: event.target.value,
      }));
      break;
    case 'color':
      setselectedModeValues((prevState: ModeBody) => ({
        ...prevState,
        color_code: newValue,
      }));
      break;
    case 'icon':
      setselectedModeValues((prevState: ModeBody) => ({
        ...prevState,
        icon: newValue,
      }));
      break;
    default:
      break;
  }
};
