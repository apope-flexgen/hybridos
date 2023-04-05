export type Actions = 'save' | 'delete';

export type SetpointActions = 'update' | 'delete';

export type Setpoints = 'variables' | 'constants';

export const SetpointTypes = ['Int', 'Float', 'Bool', 'String'];

export const CreateButtonLabel = (type: string) => `Create New ${type}`;

export type ModeBody = {
  name: string;
  color_code: string;
  icon: string;
  variables: {
    id?: string;
    name: string;
    type: string;
    unit: string;
    uri: string;
    value: number | boolean;
  }[];
  constants: {
    id?: string;
    name: string;
    type: string;
    unit: string;
    uri: string;
    value: number | boolean;
  }[];
};

export const customColorsArr = [
  'orange',
  'lightGreen',
  'teal',
  'lightBlue',
  'indigo',
  'deepPurple',
  'red',
  'pink',
  'deepOrange',
  'gray',
] as const;

export type CustomColorType = typeof customColorsArr[number];
