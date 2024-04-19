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

export const logicalOperatorList = ['==', '!=', '<', '>=', '>', '<=', '+', '-', '*', '%', '&&', '||'];
