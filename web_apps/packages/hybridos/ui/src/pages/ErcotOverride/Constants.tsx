import { Column } from '@flexgen/storybook';

export const variableOverrideColumns: Column[] = [
  { id: 'variable_name', label: 'Variable Name', minWidth: 36 },
  { id: 'ercot_standard', label: 'ERCOT Standard', minWidth: 18 },
  {
    id: 'current_value',
    label: 'Current Value',
    minWidth: 18,
    accentColor: true,
  },
  {
    id: 'enable',
    label: 'Enable',
    minWidth: 19,
    align: 'center',
  },
  {
    id: 'edit_override',
    label: 'Edit',
    minWidth: 10,
    align: 'right',
  },
  { id: 'manual_override', label: 'Manual Override', minWidth: 30 },
];

export type VariableOverrideRow = {
  id: number | string;
  variable_name: string;
  ercot_standard: string;
  current_value: string;
  enable: JSX.Element;
  edit_override: JSX.Element;
  manual_override: string | JSX.Element;
};

export const variableOverrideLabels = {
  siteSelectorTitle: 'SELECT A SITE',
  siteSelectorLabel: 'Sites',
  pageLabel: 'ERCOT OVERRIDES',
  emptyContainerMessage: 'Select a site to view and edit overrides',
  manualOverrideLabel: 'Manual Override',
};

export const SITE_NAMES_URL = '/ercot-override/sites';
export const VARIABLE_NAMES_URL = '/ercot-override/variable-names';
export const VARIABLE_VALUES_URL = '/ercot-override/variable-values';
export const UPDATE_VARIABLE_OVERRIDE_URL = '/ercot-override/override-value';
