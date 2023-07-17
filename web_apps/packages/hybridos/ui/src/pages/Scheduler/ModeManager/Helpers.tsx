// TODO: fix lint
/* eslint-disable consistent-return, max-lines */
import { v4 as uuidv4 } from 'uuid';
import { ModeBody } from './Types';

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

export const schedulerConfigLabels = {
  notifications: {
    saveSuccess: 'Mode successfully saved',
    deleteSuccess: 'Mode deleted',
    disabled: 'Editing modes is disabled while server is enabled.',
  },
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
    name: 'Name',
    color: 'Color',
    icon: 'Icon',
    newMode: 'New Mode',
    title: {
      selectedModeId: 'Mode Configuration Settings',
      noselectedModeId: 'Select Mode to View Mode Configuration Settings',
    },
    buttons: {
      cancel: 'Cancel',
      save: 'Save',
      delete: 'delete',
    },
    tooltip: {
      duplicateURI: 'Cannot have duplicate URIs within the same mode',
      invalidURI: 'Please enter a valid URI',
      defaultMode:
        'A default mode is required. It cannot be deleted, and the name cannot be edited.',
      addModeButton: 'New Mode must have a new unique name before adding more modes.',
      nameFieldHelperText: 'Mode names must be unique',
      emptyModeName: 'This field is required',
    },
    setpointRow: {
      name: 'Name',
      duplicateName: 'Duplicate Names are not allowed',
      type: 'Type',
      unit: 'Unit',
      URI: 'URI',
      value: 'Value',
      defaultValueHelper: 'Type of value entered must match Type chosen above.',
      required: 'This field is required',
      fillOutFields: '*Please fill out all required fields',
    },
  },
};

export const uriRules = "URIs must: start with '/', contain no spaces, and contain no '//'. \nURI's must be unique within a mode.";

export const handleIconColor = (icon: string, selectedModeValues: ModeBody | null | undefined) => {
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
