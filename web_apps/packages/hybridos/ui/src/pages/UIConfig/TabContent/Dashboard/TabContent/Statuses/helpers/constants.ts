export const STATUS = 'Status';
export const ADD_STATUS = 'Add Status';
export const DELETE_STATUS = 'DELETE STATUS';

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
    label: 'SourceURI',
    key: 'sourceURI',
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

export const newStatus = {
  name: '',
  scalar: '',
  units: '',
  uri: '',
};
