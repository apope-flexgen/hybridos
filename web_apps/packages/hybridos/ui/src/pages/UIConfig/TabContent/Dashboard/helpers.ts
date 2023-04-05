import { Dashboard } from 'shared/types/dtos/dashboards.dto';

export const ADD_NEW_DASHBOARD_CARD = 'ADD NEW DASHBOARD CARD';
export const DELETE_CARD = 'DELETE CARD';

export const tabOptions = [
  {
    label: 'INFO',
    value: 'info',
  },
  {
    label: 'STATUSES',
    value: 'statuses',
  },
];

export const newDashboard: Dashboard = {
  info: {},
  status: [],
};
