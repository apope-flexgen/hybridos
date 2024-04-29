import { ThemeType } from '@flexgen/storybook';

export enum ActivityLogTabs {
  Events = 'events',
  Alerts = 'alerts',
  ResolvedAlerts = 'resolved_alerts',
  AlertManagement = 'alert_management',
}

export const activityLogTabs = [
  { label: 'Events', value: ActivityLogTabs.Events },
  { label: 'Alerts', value: ActivityLogTabs.Alerts },
  { label: 'Resolved Alerts', value: ActivityLogTabs.ResolvedAlerts },
  { label: 'Alert Management', value: ActivityLogTabs.AlertManagement },
];

export const logicalOperatorList = ['==', '!=', '<', '>=', '>', '<='];

export const mainContentBoxSx = (theme: ThemeType) => ({
  display: 'flex',
  alignItems: 'flex-start',
  flexDirection: 'column',
  paddingLeft: '24px',
  paddingRight: '24px',
  paddingTop: '12px',
  borderRadius: '8px',
  height: '100%',
  overflowY: 'auto',
  width: '100%',
  boxShadow: theme.fgb.pageHeader.sizing.boxShadow,
  backgroundColor: theme.fgc.table.color.background,
});

export const tabBoxSx = {
  display: 'flex',
  paddingTop: '12px',
  height: '90%',
  width: '100%',
  borderRadius: '8px',
};
