export const SUMMARY = 'Summary';
export const ADD_SUMMARY = 'ADD SUMMARY';
export const DELETE_SUMMARY = 'DELETE SUMMARY';

export const items = [
  {
    label: 'Name',
    key: 'name',
  },
  {
    label: 'Scalar',
    helperText: 'Numbers Only',
    key: 'scalar',
    type: 'number',
  },
  {
    label: 'Units',
    key: 'units',
  },
  {
    label: 'URI',
    key: 'uri',
  },
];

export const newSummary = {
  name: '',
  scalar: '',
  units: '',
  uri: '',
};
