/* eslint-disable max-len */
import { Asset } from 'shared/types/dtos/assets.dto';

export const tabOptions = [
  {
    label: 'INFO',
    value: 'info',
  },
  {
    label: 'STATUSES',
    value: 'statuses',
  },
  {
    label: 'CONTROLS',
    value: 'controls',
  },
  {
    label: 'SUMMARY',
    value: 'summary',
  },
  {
    label: 'SUMMARY CONTROLS',
    value: 'summaryControls',
  },
  {
    label: 'ALL CONTROLS',
    value: 'allControls',
  },
];

export const CREATE_NEW_ASSET = 'CREATE NEW ASSET';
export const ADD_NEW_ASSET = 'ADD NEW ASSET';
export const SETTING = 'Setting';
export const VALUE = 'Value';
export const NO_ASSETS_TO_DISPLAY = 'No Assets to display yet.';
export const DELETE_ASSET = 'DELETE ASSET';

export const newAsset: Asset = {
  info: {},
  alarms: {
    alarmFields: [],
    faultFields: [],
  },
  allControls: [],
  controls: [],
  statuses: [],
  summary: [],
  summaryControls: [],
};

export const ASSETS_URL = '/assets';
