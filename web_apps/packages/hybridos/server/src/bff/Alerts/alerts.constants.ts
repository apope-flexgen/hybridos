import dayjs from 'dayjs';
import { ActiveAlert } from './responses/alerts.response'
import { AlertConfiguration } from './responses/alertConfig.response'

export const AlertURIs = {
  ALERT_CONFIGS: '/events/alerts/management',
  ALERT_INSTANCES: '/events/alerts',
}
export const AlertsDescriptions = {
    alertId: 'ID of particular alert',
    title: 'Title used to identify an alert',
    status: 'Status of a particular alert - inactive or active',
    expression: 'Logic behind this alert, sets up which conditions to alert upon',
    severity: 'Severity of a particular alert - 0 being lowest severity, 4 being highest',
    organization: 'Organization that a particular alert pertains to and arose from',
    site: 'Site to which a particular alert pertains',
    details: 'An array of details regarding when all conditions of the alert were met, each interior object contains a message and timestamp',
    trigger_time: 'Trigger time of a particular alert',
    resolution_message: 'Message that was given by user as they resolved this alert',
    resolution_time: 'Time at which this alert was marked as resolved',
    resolved: 'Boolean representing whether the alert has been resolved',
    deadline: 'Deadline of a particular alert - how quickly the alert must be resolved, in minutes',
    data: 'Alerts to display on page',
    count: 'Count of total alerts matching filters',
    limit: 'The maximum number of alert records the query will return,',
    severityFilter: 'Severities filter, query will return alerts with the listed severities',
    page: 'The desired page in the paginated result set',
    order: 'Order used for sorting',
    orderBy: 'Which field to order by when sorting',
    deliver: 'A boolean representing whether or not to silence the alert instances stemming from this alert configuration',
    deleted: 'A boolean flag representing whether or not this alert has been deleted',
    aliases: 'An array of objects containing the aliases and their corresponding URIs used by this alert',
    templates: 'An array of objects containing the templated wildcards and their corresponding definitions used by this alert',
    conditions: 'An array of front-end friendly expressions representing the rule logic for this alert',
    sites: 'An array of all configured sites that this alert configuration pertains to',
    name: 'The name associated with the alert',
    message: 'An error or success message returned from the events service',
    success: 'A status of whether the request was successfully returned from the events service',
    error_message: 'If alert could not be resolved, error message describes the issue that occurred'
}

export const mockedAlertConfigurationData: AlertConfiguration[] = [
  {
    id: `${Math.floor(Math.random()*90000) + 10000}`,
    enabled: true,
    severity: 0, 
    organization: 'VFakeCo',
    sites: ['Sudden Valley', 'Lima'],
    title: 'ESS 1 or ESS 2 SOC < 60%',
    last_trigger_time: dayjs().format() ,
    deadline: 30,
    aliases: [
        {
          alias: "ESS 1 SOC",
          uri: '/assets/ess/ess_1/soc',
          type: 'number',
        }, {
          alias: "ESS 2 SOC",
          uri: '/assets/ess/ess_2/soc',
          type: 'number',
        }
    ],
    conditions: [
      {
        index: 0,
        connectionOperator: null,
        operand1: {
          type: 'alias',
          value: 'ESS 1 SOC',
        },
        operator: "<",
        operand2: {
          type: 'literal',
          value: 60,
          unit: '%',
        },
      },
      {
        index: 1,
        connectionOperator: 'or',
        operand1: {
          type: 'alias',
          value: 'ESS 2 SOC',
        },
        operator: "<",
        operand2: {
          type: 'literal',
          value: 60,
          unit: '%',
        },
      }
    ]
  },
  {
    id: `${Math.floor(Math.random()*90000) + 10000}`,
    enabled: true,
    severity: 2, 
    organization: 'VFakeCo',
    sites: ['Sudden Valley', 'Lima'],
    title: 'Site Status is Shutdown',
    last_trigger_time: dayjs().format() ,
    deadline: 60,
    aliases: [
        {
          alias: "Site Status",
          uri: '/site/##/status/',
          type: 'string',
        }
    ],
    templates: [
      {
        type: "list",
        list: ['sudden_valley, lima'],
        token: '##',
      }
    ],
    conditions: [
      {
        index: 0,
        connectionOperator: null,
        operand1: {
          type: 'alias',
          value: 'Site Status',
        },
        operator: "=",
        operand2: {
          type: 'literal',
          value: "Shutdown",
        },
      }
    ]
  },
  {
    id: `${Math.floor(Math.random()*90000) + 10000}`,
    enabled: false,
    severity: 3, 
    organization: 'VFakeCo',
    sites: ['Sudden Valley', 'Lima', 'Site3', 'Site4', 'Site 5', 'Site 6'],
    title: 'ESS Reactive Power Setpoint > 2',
    last_trigger_time: dayjs().format() ,
    deadline: 60,
    aliases: [
      { alias: "ESS Reactive Power Setpoint",
        uri: '/site/ess_kVAR_cmd',
        type: 'number',
      },
    ],
    conditions: [
      {
        index: 0,
        connectionOperator: null,
        operand1: {
          type: 'alias',
          value: 'ESS Reactive Power Setpoint',
        },
        operator: ">",
        operand2: {
          type: 'literal',
          value: "2",
          unit: 'VAR'
        },
      }
    ]
  },
  {
    id: `${Math.floor(Math.random()*90000) + 10000}`,
    enabled: true,
    severity: 2, 
    organization: 'AnotherFakeCo',
    sites: ['Sudden Valley', 'Lima', 'Oxford'],
    title: 'ESS State of Health < 50%',
    last_trigger_time: dayjs().format() ,
    deadline: 60,
    aliases: [
      { alias: "ESS State of Health", uri: '/assets/ess/ess_!/soh', type: 'number'} 
    ],
    templates: [
      { type: 'sequential', to: 10, from: 1, token: '!'}
    ],
    conditions: [
      {
        index: 0,
        connectionOperator: null,
        operand1: {
          type: 'alias',
          value: 'ESS State of Health',
        },
        operator: "<",
        operand2: {
          type: 'literal',
          value: "50",
          unit: '%'
        },
        duration: {
          value: "20",
          unit: "minutes"
        }
      }
    ]
  },
]

export const mockedActiveAlertData: ActiveAlert[] = [
    {
      id: `${Math.floor(Math.random()*90000) + 10000}`,
      status: 'Active', // (1-2), representing active/inactive
      severity: 0, // (0-4), representing (info, status, alarm, fault)
      organization: 'VFakeCo',
      site: 'Sudden Valley',
      title: 'Cell Voltage Check',
      resolved: false,
      resolution_time: null,
      resolution_message: null,
      trigger_time: dayjs().format() ,
      details: 
        [
          {
              message: 'Minimum Cell Voltage: > 3.22V', 
              timestamp: dayjs().format() 
          },
          {
              message: 'Cell Voltage Delta: >.25V', 
              timestamp: dayjs().subtract(5, 'minutes').format() 
          }
        ],
      deadline: 30,
    },
    {
      id: `${Math.floor(Math.random()*90000) + 10000}`,
      status: 'Inactive', // (1-2), representing active/inactive
      severity: 1, // (0-4), representing (info, status, alarm, fault)
      organization: 'VFakeCo',
      site: 'Lima',
      resolved: false,
      resolution_time: null,
      resolution_message: null,
      trigger_time: dayjs().format() ,
      title: 'Cell Voltage Check',
      details: 
        [
          {
              message: 'Minimum Cell Voltage: > 3.22V', 
              timestamp: dayjs().format() 
          },
        ],
      deadline: 30,
    },
    {
      id: `${Math.floor(Math.random()*90000) + 10000}`,
      status: 'Inactive', // (1-2), representing active/inactive
      severity: 3, // (0-4), representing (info, status, alarm, fault)
      organization: 'AnotherFakeCo',
      site: 'Kokomo',
      resolved: false,
      resolution_time: null,
      resolution_message: null,
      title: 'All ESS Running',
      trigger_time: dayjs().subtract(5, 'minutes').format() ,
      details: 
        [
          {
              message: 'ESS Stopped Running, 9/10 Available', 
              timestamp: dayjs().subtract(5, 'minutes').format()
          },
        ],
      deadline: 120,
    },
    {
      id: `${Math.floor(Math.random()*90000) + 10000}`,
      status: 'Active', // (1-2), representing active/inactive
      severity: 2, // (0-4), representing (info, status, alarm, fault)
      organization: 'AnotherFakeCo',
      site: 'Kokomo',
      resolved: false,
      resolution_time: null,
      resolution_message: null,
      title: 'Breaker Status',
      trigger_time: dayjs().subtract(7, 'minutes').format() ,
      details: 
        [
          {
              message: 'Site Tripped: Main breaker closed', 
              timestamp: dayjs().subtract(7, 'minutes').format()
          },
        ],
      deadline: 120,
    },
    {
      id: `${Math.floor(Math.random()*90000) + 10000}`,
      status: 'Active', // (1-2), representing active/inactive
      severity: 2, // (0-4), representing (info, status, alarm, fault)
      organization: 'Allbright Energy',
      site: 'Pleasantville',
      resolved: false,
      resolution_time: null,
      resolution_message: null,
      title: 'Cell Voltage Check',
      trigger_time: dayjs().subtract(10, 'minutes').format() ,
      details: 
        [
          {
              message: 'Cell Voltage Delta: >.25V', 
              timestamp: dayjs().subtract(10, 'minutes').format()
          },
          {
            message: 'Cell Voltage Delta: >.15V', 
            timestamp: dayjs().subtract(11, 'minutes').format()
          },
          {
            message: 'Cell Voltage Delta: >.05V', 
            timestamp: dayjs().subtract(12, 'minutes').format()
          },
        ],
      deadline: 120,
    },
    {
      id: `${Math.floor(Math.random()*90000) + 10000}`,
      status: 'Active', // (1-2), representing active/inactive
      severity: 0, // (0-4), representing (info, status, alarm, fault)
      organization: 'VFakeCo',
      site: 'Pleasantville',
      resolved: false,
      resolution_time: null,
      resolution_message: null,
      title: 'Site Status Check',
      trigger_time: dayjs().subtract(11, 'minutes').format() ,
      details: 
        [
          {
              message: 'Site Tripped: Main breaker closed', 
              timestamp: dayjs().subtract(11, 'minutes').format()
          },
        ],
      deadline: 120,
    },
    {
      id: `${Math.floor(Math.random()*90000) + 10000}`,
      status: 'Active', // (1-2), representing active/inactive
      severity: 0, // (0-4), representing (info, status, alarm, fault)
      organization: 'VFakeCo',
      site: 'Sudden Valley',
      resolved: false,
      resolution_time: null,
      resolution_message: null,
      title: 'Cell Voltage Check',
      trigger_time: dayjs().subtract(12, 'minutes').format() ,
      details: 
        [
          {
              message: 'Minimum Cell Voltage: > 3.22V', 
              timestamp: dayjs().subtract(12, 'minutes').format() 
          },
          {
              message: 'Cell Voltage Delta: >.25V', 
              timestamp: dayjs().subtract(16, 'minutes').format() 
          }
        ],
      deadline: 30,
    },
    {
      id: `${Math.floor(Math.random()*90000) + 10000}`,
      status: 'Inactive', // (1-2), representing active/inactive
      severity: 1, // (0-4), representing (info, status, alarm, fault)
      organization: 'VFakeCo',
      site: 'Lima',
      resolved: false,
      title: 'Cell Voltage Check',
      resolution_time: null,
      resolution_message: null,
      trigger_time: dayjs().subtract(14, 'minutes').format() ,
      details: 
        [
          {
              message: 'Minimum Cell Voltage: > 3.22V', 
              timestamp: dayjs().subtract(14, 'minutes').format() 
          },
        ],
      deadline: 30,
    },
    {
      id: `${Math.floor(Math.random()*90000) + 10000}`,
      status: 'Inactive', // (1-2), representing active/inactive
      severity: 0, // (0-4), representing (info, status, alarm, fault)
      organization: 'AnotherFakeCo',
      site: 'Kokomo',
      resolved: false,
      resolution_time: null,
      resolution_message: null,
      title: 'All ESS Running',
      trigger_time: dayjs().subtract(16, 'minutes').format() ,
      details: 
        [
          {
              message: 'ESS Stopped Running: 9/10 Available', 
              timestamp: dayjs().subtract(16, 'minutes').format()
          },
        ],
      deadline: 120,
    },
    {
      id: `${Math.floor(Math.random()*90000) + 10000}`,
      status: 'Active', // (1-2), representing active/inactive
      severity: 1, // (0-4), representing (info, status, alarm, fault)
      organization: 'AnotherFakeCo',
      site: 'Kokomo',
      resolved: false,
      resolution_time: null,
      resolution_message: null,
      title: 'Site Status',
      trigger_time: dayjs().subtract(20, 'minutes').format() ,
      details: 
        [
          {
              message: 'Site Tripped: Main breaker closed', 
              timestamp: dayjs().subtract(20, 'minutes').format()
          },
        ],
      deadline: 30,
    },
    {
      id: `${Math.floor(Math.random()*90000) + 10000}`,
      status: 'Active', // (1-2), representing active/inactive
      severity: 2, // (0-4), representing (info, status, alarm, fault)
      organization: 'Allbright Energy',
      site: 'Pleasantville',
      resolved: false,
      resolution_time: null,
      resolution_message: null,
      title: 'Cell Voltage Check',
      trigger_time: dayjs().subtract(24, 'minutes').format() ,
      details: 
        [
          {
              message: 'Cell Voltage Delta: >.25V', 
              timestamp: dayjs().subtract(24, 'minutes').format()
          },
          {
            message: 'Cell Voltage Delta: >.15V', 
            timestamp: dayjs().subtract(26, 'minutes').format()
          },
          {
            message: 'Cell Voltage Delta: >.05V', 
            timestamp: dayjs().subtract(27, 'minutes').format()
          },
        ],
      deadline: 120,
    },
    {
      id: `${Math.floor(Math.random()*90000) + 10000}`,
      status: 'Active', // (1-2), representing active/inactive
      severity: 0, // (0-4), representing (info, status, alarm, fault)
      organization: 'VFakeCo',
      site: 'Pleasantville',
      title: 'Site Status Check',
      resolved: false,
      resolution_time: null,
      resolution_message: null,
      trigger_time: dayjs().subtract(30, 'minutes').format() ,
      details: 
        [
          {
              message: 'Site Tripped: Main breaker closed', 
              timestamp: dayjs().subtract(30, 'minutes').format()
          },
        ],
      deadline: 15,
    },
  ];