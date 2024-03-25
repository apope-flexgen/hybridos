export type Actions = 'save' | 'delete' | 'cancel';

export type SetpointActions = 'update' | 'delete';

export type Setpoints = 'variables' | 'constants';

export const SetpointTypes = ['Int', 'Float', 'Bool', 'String'];

export const CreateButtonLabel = (type: string) => `Create New ${type.slice(0, -1)}`;

export const DeleteSetpointLabel = (type: string) => `Delete ${type.slice(0, -1)}`;

export type ModeBody = {
  name: string;
  color_code: string;
  icon: string;
  variables: {
    id?: string;
    is_template?: boolean;
    batch_prefix?: string;
    batch_range?: string | string[];
    batch_value?: string | string[];
    name: string;
    type: string;
    unit: string;
    uri: string;
    value: number | boolean;
  }[];
  constants: {
    id?: string;
    name: string;
    is_template?: boolean;
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

export type CustomColorType = (typeof customColorsArr)[number];
