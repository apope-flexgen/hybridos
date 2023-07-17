export const DashboardsDescriptions = {
    dashboardsResponse: 'List of dashboard tabs from dbi.',
    addDashboardResponse: 'Add dashboard to dbi.',
    data: 'An array of data about a specific dashboard card',
    info: 'Information about the dashboard card itself',
    status: 'An array of statuses to display on the dashboard card',
    statusName: 'Name of a status to display on the dashboard card',
    statusScalar:
        'Scalar value to use for this status to display on the dashboard card',
    statusUnits: 'Units associated with this status on the dashboard card',
    statusURI: 'URI associated with this status on the dashboard card',
    infoBaseURI:
        'Second part of URI, usually the asset in question; If the URI is /assets/ess then /ess will be the base URI',
    infoBatteryView:
        'Whether or not to use the battery view in the UI for this dashboard card',
    infoBatteryViewSourceURI:
        'Which source URI to pull the battery view information from',
    batteryViewURI: 'Actual URI endpoint for battery view (usually /soc)',
    isTemplate: 'Turns on templating, usually for non summary cards',
    items: ' Only shows if IsTemplate is turned on, Allows user to define templated items (items with the same statuses)',
    name: 'Name of the dashboard card, displayed on the UI',
    sourceURIs:
        'First part of URI when determining socket source; If the URI is /assets/bms then /assets will be the source URI',
    itemName: 'Name of item to display on dashboard card',
    itemURI: 'URI associated with item on dashboard card',
}

export const DASHBOARDS_SERVICE = 'DashboardsService'
