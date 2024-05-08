export const systemStatusDescriptions = {
  dependencies: 'A list of services which depend upon this service',
  serviceName: 'Name of the Service',
  serviceStatus:
    'Status of the Service - Active, Failed, Initialized, Stopped - may include a substatus, ex: status (substatus)',
  connectionStatus: 'Connection Status for Service - Online or Offline',
  cpuUsage: 'Number representing CPU usage of Service as Percentage',
  memoryUsage: 'Number representing Memory usage of Service as Percentage',
  serviceUptime: 'Number representing the service uptime (time that the service has been running)',
  lastRestart: 'Timestring representing the last time the service was restarted',
  actionEnabled: 'Status representing whether an action button should be enabled/disabled',
  actionTitle: 'Label for this action - Start, Stop, or Restart',
  actions: 'Array of actions (start, stop, restart) that a user can use for this service',
};
