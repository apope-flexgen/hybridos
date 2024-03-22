import { Column } from '@flexgen/storybook';
import { FLEET_MANAGER } from 'src/components/BaseApp';

export const formatColumnLabel = (column: string) => column.replace(/_/g, ' ');

export const initialActiveAlertsFilters = {
  resolved: false,
  organization: 'VFakeCo',
  limit: 50,
  page: 0,
  order: -1,
  orderBy: 'trigger_time',
};

export const activeAlertsColumns = (product: string): Column[] => {
  const arrayWithBools = [
    {
      id: 'status', label: 'Status',
    },
    {
      id: 'severity', label: 'Severity',
    },
    product === FLEET_MANAGER && {
      id: 'organization', label: 'Organization',
    },
    product === FLEET_MANAGER && {
      id: 'site', label: 'Site',
    },
    {
      id: 'alert', label: 'Alert',
    },
    {
      id: 'timestamp', label: 'Last Trigger Time',
    },
    {
      id: 'deadline', label: 'Deadline',
    },
    {
      id: 'resolve', label: '',
    },
  ];
  return arrayWithBools.filter((item) => typeof item !== 'boolean' && item !== undefined) as Column[];
};
