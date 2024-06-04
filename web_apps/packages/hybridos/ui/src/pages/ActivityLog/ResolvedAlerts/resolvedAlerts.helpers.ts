import { Column } from '@flexgen/storybook';

export const formatColumnLabel = (column: string) => column.replace(/_/g, ' ');

export const initialResolvedAlertsFilters = {
  resolvedFilter: true,
  limit: 10,
  page: 0,
  order: -1,
  orderBy: 'resolution_time',
};

export const resolvedAlertsColumns = (): Column[] => {
  const arrayWithBools = [
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
      maxWidth: '300px',
    },
    {
      id: 'triggerTime',
      label: 'Trigger Time',
    },
    {
      id: 'resolutionTime',
      label: 'Resolved Time',
    },
    {
      id: 'notes',
      label: 'Notes',
    },
  ];
  return arrayWithBools.filter(
    (item) => typeof item !== 'boolean' && item !== undefined,
  ) as Column[];
};
