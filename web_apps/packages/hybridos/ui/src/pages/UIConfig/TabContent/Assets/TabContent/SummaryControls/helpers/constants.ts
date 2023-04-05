export const SUMMARY_CONTROLS = 'Summary Controls';
export const ADD_SUMMARY_CONTROL = 'ADD SUMMARY CONTROL';
export const DELETE_SUMMARY_CONTROL = 'DELETE SUMMARY CONTROL';

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

export const newSummaryControl = {
  name: '',
  scalar: '',
  units: '',
  uri: '',
  inputType: 'switch',
};
