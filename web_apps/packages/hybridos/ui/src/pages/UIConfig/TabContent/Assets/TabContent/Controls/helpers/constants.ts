export const CONTROL = 'Control';
export const ADD_CONTROL = 'ADD CONTROL';
export const DELETE_CONTROL = 'DELETE CONTROL';

export const items = [
  {
    label: 'Name',
    key: 'name',
  },
  {
    label: 'Scalar',
    helperText: 'Numbers Only',
    key: 'scalar',
  },
  {
    label: 'URI',
    key: 'uri',
  },
  {
    label: 'Input Type',
    key: 'inputType',
    select: true,
    options: [
      {
        value: 'switch',
        text: 'Switch',
      },
      {
        value: 'button',
        text: 'Button',
      },
      {
        value: 'number',
        text: 'Number',
      },
      {
        value: 'text',
        text: 'Text',
      },
    ],
  },
  {
    label: 'Units',
    key: 'units',
  },
];

export const newControl = {
  name: '',
  scalar: '',
  units: '',
  uri: '',
  inputType: 'switch',
};
