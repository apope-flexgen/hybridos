export const WebUIConfigDescriptions = {
    siteConfigurationResponse: 'Site Configuration from web ui config file',
    productResponse:
        'Product type from site configuration found in web ui config file',
    customer:
        'Customer name from site configuration found in web ui config file',
    siteName: 'Site name from site configuration found in web ui config file',
    hardware: 'Hardware/server name found in web ui config file',
}

export const SiteConfigDescriptions = {
    timezone: 'The timezone to be utilized by the user interface, used to display the time in the top app bar', 
    product: 'The HybridOS product - a product flag of “SC” or “ESS” will display HybridOS Control and a product flag of “FM” will display HybridOS Coordinate',
    gen: 'Whether or not to display the Generators page',
    ess: 'Whether or not to display the ESS page',
    solar: 'Whether or not to display the Solar page',
    feeders: 'Whether or not to display the Feeders page',
    features: 'Whether or not to display the Features page',
    site: 'Whether or not to display the Site page',
    system_status: 'Whether or not to display the System Status page',
    events: 'Whether or not to display the Events tab of the Activity Log page',
    alerting: 'Whether or not to display the Alerting tabs on the Activity Log page',
    scheduler: 'Whether or not to display the Scheduler page',
    units: 'An object containing the units to be used on the Configurable Asset Pages (generic) throughout the UI',
    inspectorComponentsName: '(deprecated)',
    site_name: 'The site name displayed in the Site Status Bar, App Bar, and the login page',
    fleet_name: 'Used to determine which fleet is using the UI, currently only being used to display the ERCOT Overrides Page (flag must be set to “ERCOT”)',
    tracker: 'Whether or not to display the Tracker page (deprecated)',
    control_cabinet: 'Whether or not to display the Control Cabinet page (deprecated)',
    met_station: 'Whether or not to display the Met Station page (deprecation)',
    fleet_manager_dashboard: 'Whether or not to display the Fleet Manager Dashboard page (deprecated)',
    site_status_bar: 'Whether or not to display the Site Status Bar component',
    customer: 'Object containing information about the customer. Displayed on the login page of the UI.Contains sub fields for customer name and server name.',
}

export const MONGO_URL = 'MONGO_URL';
export const MONGO_DB_NAME = 'MONGO_DB_NAME';
export const HTTP_TIMEOUT = 'HTTP_TIMEOUT';
export const ACCESS_TOKEN_SECRET_FIMS_SOCKET = 'ACCESS_TOKEN_SECRET_FIMS_SOCKET';
export const REFRESH_TOKEN_TIMEOUT = 'REFRESH_TOKEN_TIMEOUT';
export const ACCESS_TOKEN_TIMEOUT = 'ACCESS_TOKEN_TIMEOUT';
export const APP_SERVER_PORT = 'APP_SERVER_PORT';
export const JWT_SECRET_KEY = 'JWT_SECRET_KEY';
export const JWT_SECRET_KEY_MFA = 'JWT_SECRET_KEY_MFA';
export const JWT_SECRET_KEY_PASSWORD_EXPIRATION = 'JWT_SECRET_KEY_PASSWORD_EXPIRATION';
