import { Column } from '@flexgen/storybook';
import { FLEET_MANAGER } from 'src/components/BaseApp';

export const formatColumnLabel = (column: string) => column.replace(/_/g, ' ');

export const initialResolvedAlertsFilters = {
  resolvedFilter: true,
  orgFilter: 'VFakeCo',
  limit: 10,
  page: 0,
  order: -1,
  orderBy: 'resolution_time',
};

export const resolvedAlertsColumns = (product: string): Column[] => {
  const arrayWithBools = [
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
      id: 'triggerTime', label: 'Trigger Time',
    },
    {
      id: 'resolutionTime', label: 'Resolved Time',
    },
    {
      id: 'notes', label: 'Notes',
    },
  ];
  return arrayWithBools.filter((item) => typeof item !== 'boolean' && item !== undefined) as Column[];
};
