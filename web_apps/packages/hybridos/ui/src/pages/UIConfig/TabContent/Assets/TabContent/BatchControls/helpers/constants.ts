export const BATCH_CONTROLS = 'Batch Controls';
export const ADD_BATCH_CONTROLS = 'ADD BATCH CONTROL';
export const DELETE_BATCH_CONTROLS = 'DELETE BATCH CONTROL';

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
    label: 'URI',
    key: 'uri',
  },
  {
    label: 'Input Type',
    key: 'inputType',
    select: true,
    options: ['switch', 'button', 'number', 'text'],
  },
  {
    label: 'Units',
    key: 'units',
  },
];

export const newBatchControl = {
  name: '',
  scalar: '',
  units: '',
  uri: '',
  inputType: 'switch',
};
