/* eslint-disable max-len */
import { Asset } from 'shared/types/dtos/assets.dto';

export const tabOptions = [
  {
    label: 'Info',
    value: 'info',
  },
  {
    label: 'Statuses',
    value: 'statuses',
  },
  {
    label: 'Controls',
    value: 'controls',
  },
  {
    label: 'Summary',
    value: 'summary',
  },
  {
    label: 'Summary Controls',
    value: 'summaryControls',
  },
  {
    label: 'Batch Controls',
    value: 'batchControls',
  },
];

export const CREATE_NEW_ASSET = 'Create New Asset';
export const ADD_NEW_ASSET = 'Add New Asset';
export const SETTING = 'Setting';
export const VALUE = 'Value';
export const NO_ASSETS_TO_DISPLAY = 'No Assets to display yet.';
export const DELETE_ASSET = 'Delete Asset';

export const newAsset: Asset = {
  info: {
    name: 'New Item',
  },
  alarms: {
    alarmFields: [],
    faultFields: [],
  },
  batchControls: [],
  controls: [],
  statuses: [],
  summary: [],
  summaryControls: [],
};

export const ASSETS_URL = '/assets';
