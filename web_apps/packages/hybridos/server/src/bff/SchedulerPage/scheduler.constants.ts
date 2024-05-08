export const SchedulerDescriptions = {
  event: {
    id: 'ID of Event Object',
    duration: 'Duration of Event',
    mode: 'Mode of Event',
    startTime: 'Start Time of Event',
    variables: 'Variable object associated with Event',
    repeat: 'Repeat object associated with Event',
    repeatId: 'ID of repeat object associated with an Event',
    repeatCycle: 'Cycle of repeat object - whether event recurs weekly or daily',
    repeatFrequency: 'Frequency of repeat object - how often an event recurs',
    repeatDayMask:
      'Day mask for repeat object of event that recurs weekly - which days event occurs',
    repeatEndTime:
      'End date/time for repeating event - the date/time the event should stop recurring',
    repeatEndCount:
      'End count for repeating event - after this number of events, the event should stop recurring',
    repeatExceptions:
      'Exceptions of the repeat object - the event will not occur on dates listed in this array',
    siteId: 'ID of the Site Scheduler',
    eventId: 'ID of the specific id being added/deleted/edited',
  },
  eventsResponse:
    '<a href="https://flexgen.atlassian.net/wiki/spaces/API/pages/9371888/Scheduler+-+REST+API+HTTPS#https%3A%2F%2FIP_ADDRESS_HERE%2Fscheduler%2Fevents">Scheduler Events Rest API Documenation</a>',
  mode: {
    name: 'Name of Mode',
    colorColode: 'Color associated with mode',
    icon: 'Icon associated with mode',
    variables: 'Array of mode variables',
    constants: 'Array of mode constants',
    modeId: 'ID of the mode being deleted/edited',
  },
  modesResponse:
    '<a href="https://flexgen.atlassian.net/wiki/spaces/API/pages/9371888/Scheduler+-+REST+API+HTTPS#https%3A%2F%2FIP_ADDRESS_HERE%2Fscheduler%2Fmodes">Scheduler Modes Rest API Documenation</a>',
  setpoint: {
    id: 'ID of Setpoint',
    name: 'Name of Setpoint',
    type: 'Data type of setpoint',
    unit: 'Unit of setpoint data',
    uri: 'URI associated with setpoint',
    value: 'Value of setpoint data',
  },
  configuration: {
    schedulerType:
      'Type of scheduler for configuration - FM (fleet manager) or SC (site controller)',
    localSchedule:
      'Series of fields associated with the local schedule - optional on FM, required on SC',
    localScheduleId: 'ID of the local schedule',
    localScheduleName: 'Display name of the local schedule',
    localScheduleClothed:
      'True/False value representing whether to send setpoints as clothed body requests',
    localScheduleSetPointEnfocement: 'Object used to configure the Setpoint Enforcement feature',
    setpointEnabled:
      'When true, Scheduler will send GETs to event setpoints during the duration of the event. If a value found in a GET reply does not match its setpoint value, a fresh SET will be sent to the setpoint’s URI.',
    setpointFreqSeconds: 'How many seconds between setpoint enforcement GETs being sent out.',
    webSocketsObject: 'Object used to configure Fleet-Site interface feature',
    webSocketsClients:
      'An array, where each element of the array is an object that represents a Site Scheduler',
    webSocketClientId:
      'The ID of the Site Scheduler. Used in URIs, so spaces and slashes are not allowed.',
    webSocketClientName: 'Display name of the Site Scheduler',
    webSocketClientIp: 'IP of the Site Scheduler’s server',
    webSocketClientPort: 'The port that the Site Scheduler’s HTTPS WebSocket server is hosted on',
    webSocketServers: 'Fields associated with the server in the Fleet-Site interface',
    webSocketServerEnabled:
      'When true, an HTTPS WebSocket will be served to which a Fleet Scheduler can connect',
    webSocketServerPort: 'The port that the HTTPS WebSocket server should be hosted on.',
    scada:
      'Fields used to configure the SCADA interface to communicate with Scheduler over Modbus or DNP3',
    scadaStageSize: 'The number of event slots in the Staging Area',
    scadaMaxNumConfigs: 'The number of event slots the SCADA interface will maintain',
    scadaNumFloats: 'The number of float variables that will be represented per event slot',
    scadaNumInts: 'The number of integer variables that will be represented per event slot',
    scadaNumBools: 'The number of boolean variables that will be represented per event slot',
    scadaNumStrings: 'The number of string variables that will be represented per event slot',
  },
  configurationResponse:
    '<a href="https://flexgen.atlassian.net/wiki/spaces/API/pages/9371888/Scheduler+-+REST+API+HTTPS#https%3A%2F%2FIP_ADDRESS_HERE%2Fscheduler%2Fconfiguration">Scheduler Configuration Rest API Documenation</a>',
  timezonesResponse:
    '<a href="https://flexgen.atlassian.net/wiki/spaces/API/pages/9371888/Scheduler+-+REST+API+HTTPS#https%3A%2F%2FIP_ADDRESS_HERE%2Fscheduler%2Ftimezones">Scheduler Timezones Rest API Documenation</a>',
  connectedResponse:
    '<a href="https://flexgen.atlassian.net/wiki/spaces/API/pages/9371888/Scheduler+-+REST+API+HTTPS#https%3A%2F%2FIP_ADDRESS_HERE%2Fscheduler%2Fconnected">Scheduler Connected Rest API Documenation</a>',
};
