import { SeverityType } from '@flexgen/storybook';

export type SeveritiesStateObject = {
  [Key in SeverityType]: boolean;
};

export type CheckBoxColors = 'error' | 'warning' | 'primary' | 'secondary' | 'info' | 'success';

export type SeverityProperties = {
  [Key in SeverityType]: {
    color?: string;
    label: Key;
  };
};
