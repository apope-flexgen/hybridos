export const SiteStatusDescriptions = {
  data: 'Data object containing all information being passed to the Site Status Bar - label, active faults, active alarms, site status, and data points',
  siteName:
    'Label to display on the left hand side of the site status bar, comes from site_status.json config file',
  activeFaults:
    'A count of the total number of active faults on the site, comes from /site/summary fims response, active_faults field',
  activeAlarms:
    'A count of the total number of active alarms on the site, comes from /site/summary fims response, active_alarms field',
  siteState:
    'The site status to display on the right hand side of the siste status bar, comes from /site/summary fims response, site_status field',
  baseData: 'Object containing data used on the site status bar',
  dataPoints:
    'Object containing data points to display in the site status bar, comes from the site_status.json config file',
  dataPointsDto:
    'Object containing sub-objects, each of which represents a single data point to display in the site status bar',
  dataPointLabel: 'Label to display below data point, describes the data being displayed',
  dataPointValue: 'Value of specific data point',
  dataPointUnit:
    'Unit of specific data point, will be displayed next to the value ont he site status bar',
};
