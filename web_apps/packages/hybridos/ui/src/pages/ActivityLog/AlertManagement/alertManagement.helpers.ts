import { Column } from '@flexgen/storybook';
import { FLEET_MANAGER } from 'src/components/BaseApp';
import { AlertConfigurationObject } from 'src/pages/ActivityLog/activityLog.types';

export type AlertManagementPageLayout = 'table' | 'form';

export const AlertManagementPageLayouts: {
  [key: string]: AlertManagementPageLayout
} = {
  TABLE: 'table',
  FORM: 'form',
};

export const initialAlertManagementFilters = {
  limit: 50,
  page: 0,
  order: -1,
  orderBy: 'lastUpdated',
};

export const determineAlertManagementTable = (
  alertManagementView: 'table' | 'form',
  alertFormValues?: AlertConfigurationObject | null,
) => {
  if (alertManagementView === 'table') return 'Alert Management';
  if (alertFormValues?.id) return 'Edit Alert';
  return 'Create New Alert';
};

export const alertManagementColumns = (product: string): Column[] => {
  const arrayWithBools = [
    {
      id: 'deliver', label: 'Deliver',
    },
    {
      id: 'name', label: 'Alert Name',
    },
    product === FLEET_MANAGER && {
      id: 'organization', label: 'Organization',
    },
    product === FLEET_MANAGER && {
      id: 'sites', label: 'Sites',
    },
    {
      id: 'severity', label: 'Severity',
    },
    {
      id: 'last_triggered', label: 'Last Triggered',
    },
    {
      id: 'deadline', label: 'Required Response Time',
    },
    {
      id: 'actions', label: 'Actions',
    },
  ];
  return arrayWithBools.filter((item) => typeof item !== 'boolean' && item !== undefined) as Column[];
};
