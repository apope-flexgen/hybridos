import { Column } from '@flexgen/storybook';
import { FLEET_MANAGER } from 'src/components/BaseApp';
import { AlertFilters } from 'src/pages/ActivityLog/activityLog.types';

export const formatColumnLabel = (column: string) => column.replace(/_/g, ' ');

export const initialActiveAlertsFilters: AlertFilters = {
  resolvedFilter: false,
  limit: 10,
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
