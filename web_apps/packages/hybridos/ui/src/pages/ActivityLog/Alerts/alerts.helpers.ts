import { Column } from '@flexgen/storybook';

import { AlertFilters } from 'src/pages/ActivityLog/activityLog.types';

export const formatColumnLabel = (column: string) => column.replace(/_/g, ' ');

export const initialActiveAlertsFilters: AlertFilters = {
  resolvedFilter: false,
  order: -1,
  orderBy: 'trigger_time',
};

export const activeAlertsColumns = (): Column[] => {
  const arrayWithBools = [
    {
      id: 'status',
      label: 'Status',
    },
    {
      id: 'severity',
      label: 'Severity',
    },
    {
      id: 'organization',
      label: 'Organization',
    },
    {
      id: 'alert',
      label: 'Alert',
    },
    {
      id: 'timestamp',
      label: 'Last Trigger Time',
    },
    {
      id: 'deadline',
      label: 'Deadline',
    },
    {
      id: 'resolve',
      label: '',
    },
  ];
  return arrayWithBools.filter(
    (item) => typeof item !== 'boolean' && item !== undefined,
  ) as Column[];
};
