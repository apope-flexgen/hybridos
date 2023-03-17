/* eslint-disable camelcase */
import { siteConfiguration, doUnauthorizedLogout } from './AppAuth';

const events = require('events');

export const keyboardEventEmitter = new events.EventEmitter();
export const alertComponentsEventEmitter = new events.EventEmitter();
// eslint-disable-next-line global-require
export const FLEXGEN_LOGO = require('./img/FlexGen_Primary Logo - Black.png');
export const FLEXGEN_LOGO2 = require('./img/FlexGen_Product Logo - Gradient.png');

export const WEBUI_CONFIG_PATH = '/web_configuration';
export const LOGIN_PAGE_PATH = '/login';
export const DASHBOARD_PAGE_PATH = '/dashboard';
export const FLEET_MANAGER_DASHBOARD_PAGE_PATH = '/fleetmanagerdashboard';
export const SITE_PAGE_PATH = '/site';
export const FEATURES_PAGE_PATH = '/features';
export const EVENTS_PAGE_PATH = '/events';
export const ESS_PAGE_PATH = '/assets/ess';
export const GENERATOR_PAGE_PATH = '/assets/generators';
export const SOLAR_PAGE_PATH = '/assets/solar';
export const FEEDERS_PAGE_PATH = '/assets/feeders';
export const CONTROL_CABINET_PAGE_PATH = '/control_cabinet';
export const USER_ADMINISTRATION_PAGE_PATH = '/administration/useradministration';
export const SITE_ADMINISTRATION_PAGE_PATH = '/administration/siteadministration';
export const COMPONENTS_PAGE_PATH = '/inspector/components';
export const FIMS_PAGE_PATH = '/inspector/fims';
export const BMS_PAGE_PATH = '/assets/bms';
export const PCS_PAGE_PATH = '/assets/pcs';
export const SCHEDULER_PAGE_PATH = '/schedule';
export const DBI_DOWNLOAD_PAGE_PATH = '/inspector/dbiDownload';
export const SYSTEM_INFORMATION_PAGE_PATH = 'inspector/systemInformation';
export const SCHEDULER_CONFIGURATION_PATH = '/SchedulerConfiguration';
export const VAR_OVERRIDE_PAGE_PATH = '/variableOverride';

export const isLoading = true;

// // NOTES ABOUT SOCKET.IO:
// // socket.io uses long-polling ("polling") and websockets as its transports.
// // Performance with polling is pretty good, but websockets is noticeably
// // faster and puts less load on the server so it is strongly preferred.
// // The catch is that development requires polling and in production, a
// // proxy or firewall might *disallow* websockets (again, requiring polling).

// // socket.io always tries polling first and then websockets. There is no
// // simple way to get it to use websockets first. Their own documentation
// // says that if you want websockets first, you should disallow polling
// // by setting the transports to ['websocket'] only.

// // If you do that but still need to allow for a possible fallback to polling
// // then you need to look for a 'reconnect_attempt' message and then reset
// // your transports configuration to ['polling', 'websocket']. IMPORTANT
// // NOTE: The initial receipt of the 'reconnect_attempt' message takes a long
// // time (15-20 seconds or more) but then all the sockets will subsequently
// // use polling and will be reasonably quick.

// // In Layout, we try listening for web_server's heartbeat at startup. If we
// // hear the heartbeat then the socket transports are working and we turn the
// // heartbeat socket off. If we DO NOT hear the heartbeat then we will
// // eventually receive a reconnect_attempt message in ".on('reconnect_attempt'"
// // and reset the transports to use polling. Then we turn off the heartbeat; its
// // job is done for now.

// // to test the code below, set nodeEnvIsDevelopment to false and then view the
// // site in a development environment (e.g., React `sudo npm start`). Websocket
// // is not available in a development environment so the site will be forced to
// // fallback to polling. The initial receipt of the 'reconnect_attempt' message
// // takes a long time (15-20 seconds) but then all the sockets will
// // subsequently use polling and will be reasonably quick.

// // Don't forget to set nodeEnvIsDevelopment back to the NODE_ENV check when
// // you are done testing!

// // export const nodeEnvIsDevelopment = false;
// export const nodeEnvIsDevelopment = process.env.NODE_ENV === 'development';

// // socket.io transports default to ['polling', 'websocket'] (in that order)
// // when not otherwise specified (as in the middle part of the ternary
// // expressions below).
/* eslint-disable max-len */
// export const socket_site = nodeEnvIsDevelopment ? io('/site', { secure: true }) : io('/site', { secure: true, transports: ['websocket'] });
// export const socket_features = nodeEnvIsDevelopment ? io('/features', { secure: true }) : io('/features', { secure: true, transports: ['websocket'] });
// export const socket_events = nodeEnvIsDevelopment ? io('/events', { secure: true }) : io('/events', { secure: true, transports: ['websocket'] });
// export const socket_assets = nodeEnvIsDevelopment ? io('/assets', { secure: true }) : io('/assets', { secure: true, transports: ['websocket'] });
// export const socket_components = nodeEnvIsDevelopment ? io('/components', { secure: true }) : io('/components', { secure: true, transports: ['websocket'] });
// export const socket_administration = nodeEnvIsDevelopment ? io('/administration', { secure: true }) : io('/administration', { secure: true, transports: ['websocket'] });
// export const socket_inspector = nodeEnvIsDevelopment ? io('/inspector', { secure: true }) : io('/inspector', { secure: true, transports: ['websocket'] });
// export const socket_heart1000 = nodeEnvIsDevelopment ? io('/heart1000', { secure: true }) : io('/heart1000', { secure: true, transports: ['websocket'] });
// export const socket_serverCount = nodeEnvIsDevelopment ? io('/serverCount', { secure: true }) : io('/serverCount', { secure: true, transports: ['websocket'] });
/* eslint-enable max-len */

export async function updatePropertyValue(uri, property, value) {
    const response = await fetch(`/api/${uri}/${property}/${value}`, {
        method: 'POST',
        credentials: 'include',
    });
    if (response.status === 401) {
        doUnauthorizedLogout();
        return null;
    }
    return response;
}

export async function getDataForURI(api_endpoint, noPrefix) {
    // no prefix allows special routing for inspector queries
    const theAPIEndpoint = noPrefix === true ? `/${api_endpoint}` : `/api/${api_endpoint}`;
    const response = await fetch(theAPIEndpoint, {
        credentials: 'include',
        headers : { 
            'Accept': 'application/json'
           }    
    });
    if (response.status === 401) {
        doUnauthorizedLogout();
        return null;
    }
    return response;
}
export function doGetPropertiesByUITypeCoercions(internal_object, key, ui_type) {
    if ((typeof internal_object.value) === 'number'
        && internal_object.ui_type !== 'fault'
        && internal_object.ui_type !== 'alarm') {
        /* eslint-disable no-param-reassign */
        internal_object.realValue = internal_object.value * internal_object.scaler;
        internal_object.unitPrefix = siteConfiguration.units[internal_object.unit]
            ? siteConfiguration.units[internal_object.unit] : '';
        switch (internal_object.unitPrefix) {
            case 'k':
                // divide by 1,000
                internal_object.displayValue = (internal_object.realValue / 1000).toFixed(2);
                break;
            case 'M':
                // divide by 1,000,000
                internal_object.displayValue = (internal_object.realValue / 1000000).toFixed(2);
                break;
            default:
                internal_object.displayValue = (internal_object.realValue).toFixed(2);
        }
    } else if (internal_object.ui_type) {
        internal_object.displayValue = internal_object.value;
        internal_object.unitPrefix = '';
    }
    if (internal_object.ui_type === ui_type) {
        internal_object.id = key;
    }
    /* eslint-enable no-param-reassign */
    return internal_object;
}

export function getPropertiesByUIType(asset, ui_type) {
    let asset_ui_type_objects = [];
    Object.keys(asset).forEach((key) => {
        const internal_object = asset[key];
        if (Array.isArray(internal_object)) { // this is the case for components
            Object.keys(internal_object).forEach((part, i) => {
                doGetPropertiesByUITypeCoercions(internal_object[i], null, null);
            });
        } else { // this is the case for assets, Dashboard, Site, and Features
            const internal_object_coerced = doGetPropertiesByUITypeCoercions(internal_object,
                key, ui_type);
            if ((ui_type === 'fault' || ui_type === 'alarm') && internal_object_coerced.length === 0) {
                asset_ui_type_objects = [{
                    value: 0,
                }];
            }
            if (internal_object_coerced.ui_type === ui_type) {
                internal_object_coerced.id = key;
                asset_ui_type_objects.push(internal_object_coerced);
            }
        }
    });
    return asset_ui_type_objects;
}

export function formatSingleAsset(asset, base_uri, assetType) {
    const status = getPropertiesByUIType(asset, 'status');
    const control = getPropertiesByUIType(asset, 'control');
    const faults = getPropertiesByUIType(asset, 'fault');
    const alarms = getPropertiesByUIType(asset, 'alarm');
    status.forEach((obj) => {
        /* eslint-disable no-param-reassign */
        obj.base_uri = base_uri;
        obj.category = assetType;
        obj.asset_id = asset.id;
        obj.api_endpoint = obj.id;
    });
    control.forEach((obj) => {
        obj.base_uri = base_uri;
        obj.category = assetType;
        obj.asset_id = asset.id;
        obj.api_endpoint = obj.id;
        /* eslint-enable no-param-reassign */
    });
    const formatted_object = {
        id: asset.id,
        name: asset.name,
        uri: `${base_uri}/${assetType}/${asset.id}`,
        status,
        controls: control,
        faults,
        alarms,
    };
    return formatted_object;
}

export async function setOrPutDataForURI(api_endpoint, body, setOrPut) {
    const theAPIEndpoint = `/api/${api_endpoint}`;
    const response = await fetch(theAPIEndpoint, {
        method: setOrPut,
        body,
        headers: { 'Content-Type': 'application/json' },
        credentials: 'include',
    });
    if (response.status === 401) {
        doUnauthorizedLogout();
        return null;
    }
    return response;
}

export function formatAssetObjects(data, base_uri, assetType) {
    const asset_ids = Object.keys(data);
    const formatted_objects = [];
    asset_ids.map((asset_id) => {
        const thisAsset = data[asset_id];
        thisAsset.id = asset_id;
        const result = formatSingleAsset(thisAsset, base_uri, assetType);
        formatted_objects.push(result);
        return null;
    });
    return formatted_objects;
}
export function formatSingleFeatureSiteObject(object, base_uri, category) {
    const status = getPropertiesByUIType(object, 'status');
    const control = getPropertiesByUIType(object, 'control');
    const faults = getPropertiesByUIType(object, 'fault');
    const alarms = getPropertiesByUIType(object, 'alarm');
    status.forEach((obj) => {
        /* eslint-disable no-param-reassign */
        obj.base_uri = base_uri;
        obj.category = category;
        obj.api_endpoint = obj.id;
    });
    control.forEach((obj) => {
        obj.base_uri = base_uri;
        obj.category = category;
        obj.api_endpoint = obj.id;
        /* eslint-enable no-param-reassign */
    });
    const formatted_object = {
        id: object.id,
        name: object.name,
        uri: `${base_uri}/${category}`,
        status,
        controls: control,
        faults,
        alarms,
        category,
    };
    return formatted_object;
}
export function formatFeatureSiteObjects(data, base_uri) {
    const category_ids = Object.keys(data);
    const formatted_objects = [];
    category_ids.map((category) => {
        const thisObject = data[category];
        thisObject.id = category;
        const result = formatSingleFeatureSiteObject(thisObject, base_uri, category);
        formatted_objects.push(result);
        return null;
    });
    return formatted_objects;
}