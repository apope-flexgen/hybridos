export const ALL_CONTROLS = 'All Controls';
export const ADD_ALL_CONTROLS = 'ADD ALL CONTROLS';
export const DELETE_ALL_CONTROLS = 'DELETE ALL CONTROLS';

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

export const newAllControl = {
  name: '',
  scalar: '',
  units: '',
  uri: '',
  inputType: 'switch',
};
