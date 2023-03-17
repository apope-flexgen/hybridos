/* eslint-disable no-alert */
/* eslint-disable no-use-before-define */
/* eslint-disable camelcase */
import io from 'socket.io-client';

// Don't forget to set nodeEnvIsDevelopment back to the NODE_ENV check when
// you are done testing!
// export const nodeEnvIsDevelopment = false;
export const nodeEnvIsDevelopment = process.env.NODE_ENV === 'development';
if (nodeEnvIsDevelopment) console.log('nodeEnvIsDevelopment:', nodeEnvIsDevelopment);

/* eslint-disable import/no-mutable-exports */
export let socket_auth;
export let socket_inspectorControl;
export let socket_site;
export let socket_sites;
export let socket_features;
export let socket_events;
export let socket_assets;
export let socket_components;
export let socket_administration;
export let socket_inspector;
export let socket_heart1000;
export let socket_serverCount;
export let socket_scheduler;

export const siteConfiguration = {};
export let accessPerRole = {}; // describes access for every role
export let roleAccess; // describes access for the logged-in role
export let inspectorMode;
export let userUsername;
export let userRole;
/* eslint-enable import/no-mutable-exports */

/**
 * Used console log only when in development
 * @param {string} message message to log
 */
export function consoleLog(message) {
    if (nodeEnvIsDevelopment) console.log(message);
}

/**
 * Converts site configuration object from file to web_ui friendly form
 * @param {object} object contains siteConfiguration
 */
function doSiteConfigurationConversion(object) {
    // this is required at present because the config file and the variables in web_ui don't jibe.
    const theConfig = object.siteConfiguration;
    siteConfiguration.timezone = theConfig.timezone ? theConfig.timezone : 'America/New_York';
    siteConfiguration.siteName = theConfig.site_name;
    siteConfiguration.units = theConfig.units;
    siteConfiguration.includeSite = theConfig.site;
    siteConfiguration.includeFleetManagerDashboard = theConfig.fleet_manager_dashboard;
    siteConfiguration.includeFeatures = theConfig.features;
    siteConfiguration.includeEvents = theConfig.events;
    siteConfiguration.includeStorage = theConfig.ess;
    siteConfiguration.includeGenerator = theConfig.gen;
    siteConfiguration.includeSolar = theConfig.solar;
    siteConfiguration.includeFeeders = theConfig.feeders;
    siteConfiguration.includeGrid = theConfig.feeders;
    siteConfiguration.includeControlCabinet = theConfig.control_cabinet;
    siteConfiguration.includeMetStation = theConfig.met_station;
    siteConfiguration.includeTracker = theConfig.tracker;
    siteConfiguration.includeBMS = theConfig.essController;
    siteConfiguration.includePCS = theConfig.essController;
    siteConfiguration.inspectorComponentsName = theConfig.inspectorComponentsName;
    siteConfiguration.metadata = theConfig.metadata;
    siteConfiguration.setGrid = theConfig.setGrid;
    siteConfiguration.includeScheduler = theConfig.scheduler;
    siteConfiguration.primaryStatus = theConfig.primaryStatuses;
    siteConfiguration.fleetManager = theConfig.fleet_manager;
    siteConfiguration.fleetName = theConfig.fleet_name;
}

/**
 * Confirms role access for URIs
 * @param {string} accessPoint URI to check access
 */
export function confirmRoleAccess(accessPoint) {
    if (accessPerRole && userRole && accessPoint) {
        // confirms that the requesting role is allowed to
        // access the accessPoint (usually a uri). If not,
        // we invalidate and disconnect the JWT
        // TODO: currently all UI-related role access is 'readWrite', this will change in future
        // if we want or need to differentiate between 'read' and 'readWrite' in the UI.
        if ((accessPerRole[userUsername] && accessPerRole[userUsername].indexOf(`${accessPoint} readWrite`) > -1) || accessPerRole[userRole].indexOf(`${accessPoint} readWrite`) > -1) {
            consoleLog(`ROLE ACCESS to ${accessPoint}: valid`);
            return true;
        }
        consoleLog(`ROLE ACCESS to ${accessPoint}: INVALID`);
        return false;
    }
    return false;
}

/**
 * Performs login check and boots user if invalid
 * @param {string} username username
 * @param {string} password password
 * @param {function} callback post processing function for components
 */
export function doLogin(username, password, callback) {
    if (username.length > 4 && password.length > 4) {
        fetch(`/login`, {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            credentials: 'include',
            body: JSON.stringify({username, password})
        })
            .then((response) => response.json())
            .then((response) => {
                
                if (response.role) {
                    if (response.message) { // login error
                        consoleLog(response.message);
                        consoleLog('API: doLogin unauthorized');
                        doUnauthorizedLogout();
                    } else if (response.requiredAuth.length === 0) {
                        // console.log('AUTH: client successfully authenticated, issued JWT
                        // authorization', username, response.role);
                        consoleLog('AUTH: client successfully authenticated, issued JWT authorization');
                        userUsername = username;
                        userRole = response.role;
                        // Why do we do this timeout? For a good reason. Read on...
                        // socket.io apparently goes with whatever JWT it first gets. Even
                        // if the JWT gets updated, socket.io continues to use the original
                        // JWT. If allowed to connect right away, socket.io will use any
                        // JWT left over from a previous session. JWTs are saved as cookies
                        // in the browser and because the cookies we use are "httponly"
                        // there is no way to access or delete or modify them via JavaScript.
                        // A browser - trying to be helpful - will populate a new window or
                        // tab with a JWT from a previous session. This is no problem for
                        // authentication and authorization (the invalidated JWT is easily
                        // detected) but causes trouble when the socket keeps trying to connect
                        // with an outdated JWT. So, we wait until we are sure we've received
                        // the newly-issued JWT. There is no practical way to programmatically
                        // detect whether we have received the new JWT so we simply wait
                        // two seconds before initiating the socket. My estimation is that
                        // the new JWT is inserted in, perhaps, a quarter second, so this
                        // is a prudent amount of time, in my opinion. (You can see the
                        // newly-issued JWT appear as-it-happens in the Application tab
                        // of a Chrome browser's developer console.) -DM 5/30/2020
                        setTimeout(() => {
                            openSockets(callback);
                        }, 2000);
                    } else if (response.requiredAuth.includes("password_expiration")){
                        
                        callback({
                            message: "password_expired", 
                            user_state_crypto: response.user_state_crypto, 
                            username: response.username, 
                            role: response.role,
                            mfa: response.mfa,
                            requiredAuth: response.requiredAuth,
                        })
                    } else if (response.requiredAuth.includes("multi_factor_authentication")){
                        callback({
                            message: "multi_factor_authentication", 
                            user_state_crypto: response.user_state_crypto, 
                            username: response.username, 
                            role: response.role,
                            mfa: response.mfa,
                            secret_key: response.secret_key,
                            requiredAuth: response.requiredAuth,
                        })
                    } else {
                        doUnauthorizedLogout();
                    }
                } else {
                    doUnauthorizedLogout();
                }
                
            }).catch((error) => {
                consoleLog('API: login error: ');
                consoleLog(error);
                doUnauthorizedLogout();
            });
    } else {
        doUnauthorizedLogout();
    }
}

// logs in user who has authenticated using MFA, Password Expiration (login modals)
export function superAuthLogin(username, role, callback){
    consoleLog('AUTH: client successfully authenticated, issued JWT authorization');
    userUsername = username;
    userRole = role;
   
    setTimeout(() => {
        openSockets(callback);
    }, 2000);
}

// if a user opens route "/" and they have a non-expired, valid jwt cookie, user set up and sockets open
export function tokenLogin(user, role, callback) { 
    userRole = role;
    userUsername = user;
    openSockets(callback); 
}
/**
 * Connects to back end sockets
 * @param {function} callback post processing function for components
 */
function openSockets(callback) {
    // console.log('>>>>>>> opening sockets');
    // NOTES ABOUT SOCKET.IO:
    // socket.io uses long-polling ("polling") and websockets as its transports.
    // Performance with polling is pretty good, but websockets is noticeably
    // faster and puts less load on the server so it is strongly preferred.
    // The catch is that development requires polling and in production, a
    // proxy or firewall might *disallow* websockets (again, requiring polling).

    // socket.io always tries polling first and then websockets. There is no
    // simple way to get it to use websockets first. Their own documentation
    // says that if you want websockets first, you should disallow polling
    // by setting the transports to ['websocket'] only.

    // If you do that but still need to allow for a possible fallback to polling
    // then you need to look for a 'reconnect_attempt' message and then reset
    // your transports configuration to ['polling', 'websocket']. IMPORTANT
    // NOTE: The initial receipt of the 'reconnect_attempt' message takes a long
    // time (15-20 seconds or more) but then all the sockets will subsequently
    // use polling and will be reasonably quick.

    // In Layout, we try listening for web_server's heartbeat at startup. If we
    // hear the heartbeat then the socket transports are working and we turn the
    // heartbeat socket off. If we DO NOT hear the heartbeat then we will
    // eventually receive a reconnect_attempt message in ".on('reconnect_attempt'"
    // and reset the transports to use polling. Then we turn off the heartbeat; its
    // job is done for now.

    // to test the code below, set nodeEnvIsDevelopment to false and then view the
    // site in a development environment (e.g., React `sudo npm start`). Websocket
    // is not available in a development environment so the site will be forced to
    // fallback to polling. The initial receipt of the 'reconnect_attempt' message
    // takes a long time (15-20 seconds) but then all the sockets will
    // subsequently use polling and will be reasonably quick.

    // socket.io transports default to ['polling', 'websocket'] (in that order)
    // when not otherwise specified (as in the middle part of the ternary
    // expressions below).
    socket_auth = nodeEnvIsDevelopment ? io('/api', { secure: true }).connect() : io('/api', { secure: true, transports: ['websocket'] }).connect();
    socket_inspectorControl = nodeEnvIsDevelopment ? io('/inspectorControl', { secure: true }) : io('/inspectorControl', { secure: true, transports: ['websocket'] });
    // eslint-disable-next-line no-unused-vars
    socket_inspectorControl.on('message', (data) => {
        // console.log('>>>>>>> socket_inspectorControl message received');
        // console.log(data.message);
    });
    socket_auth.on('siteConfiguration', (object) => {
        // console.log('>>>>>>> socket_auth siteConfiguration received');
        doSiteConfigurationConversion(object);
    });

    socket_auth.on('accessPerRole', (object) => {
        // console.log('>>>>>>> socket_auth accessPerRole received');
        accessPerRole = Object.assign(accessPerRole, object.accessPerRole);
        roleAccess = { ...accessPerRole[userRole] };
        callback(userUsername, userRole);
    });
    socket_auth.on('message', (msg) => {
        // console.log('>>>>>>> socket_auth message received');
        consoleLog(msg.message);
        consoleLog(msg);
        if (userRole === 'admin') {
            socket_auth.emit('message', { message: 'client acknowledges the successful socket.io connection' });
        }
    });
    socket_auth.on('unauthorized', (msg) => {
        // console.log('>>>>>>> socket_auth unauthorized received');
        consoleLog(msg.message);
        doUnauthorizedLogout();
    });
    socket_site = nodeEnvIsDevelopment ? io('/site', { secure: true }) : io('/site', { secure: true, transports: ['websocket'] });
    socket_sites = nodeEnvIsDevelopment ? io('/sites', { secure: true }) : io('/sites', { secure: true, transports: ['websocket'] });
    socket_features = nodeEnvIsDevelopment ? io('/features', { secure: true }) : io('/features', { secure: true, transports: ['websocket'] });
    socket_events = nodeEnvIsDevelopment ? io('/events', { secure: true }) : io('/events', { secure: true, transports: ['websocket'] });
    socket_assets = nodeEnvIsDevelopment ? io('/assets', { secure: true }) : io('/assets', { secure: true, transports: ['websocket'] });
    socket_components = nodeEnvIsDevelopment ? io('/components', { secure: true }) : io('/components', { secure: true, transports: ['websocket'] });
    socket_administration = nodeEnvIsDevelopment ? io('/administration', { secure: true }) : io('/administration', { secure: true, transports: ['websocket'] });
    socket_inspector = nodeEnvIsDevelopment ? io('/inspector', { secure: true }) : io('/inspector', { secure: true, transports: ['websocket'] });
    socket_heart1000 = nodeEnvIsDevelopment ? io('/heart1000', { secure: true }) : io('/heart1000', { secure: true, transports: ['websocket'] });
    socket_serverCount = nodeEnvIsDevelopment ? io('/serverCount', { secure: true }) : io('/serverCount', { secure: true, transports: ['websocket'] });
    socket_scheduler = nodeEnvIsDevelopment ? io('/scheduler', { secure: true }) : io('/scheduler', { secure: true, transports: ['websocket'] });
}

/**
 * Regular logout without error notification
 * @param {object} event prevent default notifications
 */
export function doLogout(event) {
    if (event) event.preventDefault();
    fetch('/api/logout', {
        method: 'POST',
        credentials: 'include',
    })
        .then((response) => response.json())
        .then((response) => {
            consoleLog('API - logout response: ');
            consoleLog(response.message);
            window.location.href = '/';
        }).catch((error) => {
            consoleLog('API - logout error: ');
            consoleLog(error);
        });
}

/**
 * Alert user of anauthorized access then logout
 */
export function doUnauthorizedLogout() {
    // eslint-disable-next-line no-alert
    alert('API: unauthorized');
    doLogout();
}

/**
 * Creates new user with passed credentials
 * @param {string} username username
 * @param {string} password password
 * @param {string} role role
 */
export function createUser(username, password, role) {
    fetch(`/api/createuser?username=${encodeURIComponent(username)}&password=${encodeURIComponent(password)}&role=${encodeURIComponent(role)}`, {
        method: 'POST',
        credentials: 'include',
    })
        .then((response) => response.json())
        .then((result) => {
            consoleLog(`API - createUser response: ${result.message}`);
            try {
                document.getElementById('consoleDisplay').innerHTML = result.message;
                /* eslint-disable no-empty */
            } catch (err) { }
        })
        .catch((error) => {
            consoleLog('API - createUser error: ');
            consoleLog(error);
            try {
                document.getElementById('consoleDisplay').innerHTML = `API - createUser error: ${error}`;
            } catch (err) { }
        });
}

/**
 * Removes user based on username
 * @param {string} username username
 */
export function removeUser(username) {
    if (username !== userUsername) {
        fetch(`/api/removeuser?username=${encodeURIComponent(username)}`, {
            method: 'POST',
            credentials: 'include',
        })
            .then((response) => response.json())
            .then((response) => {
                consoleLog(`API - removeUser response: ${response.message}`);
                return response;
                // try {
                //     document.getElementById('consoleDisplay').innerHTML = result.message;
                // } catch (err) { }
            })
            .catch((error) => {
                consoleLog('API - removeUser error: ');
                consoleLog(error);
                try {
                    document.getElementById('consoleDisplay').innerHTML = `API - removeUser error: ${error}`;
                } catch (err) { }
            });
    } else {
        consoleLog('API ERROR: cannot remove active user');
        try {
            document.getElementById('consoleDisplay').innerHTML = 'API ERROR: cannot remove active user';
        } catch (err) { }
        /* eslint-enable no-empty */
    }
}

/**
 * Edit user based on username
 * @param {string} username username
 * @param {Object} ...args accepts infinite number of Object arguments
 * example below, wants to edit password for user "bob"
 * editUser("bob", {name: "password": value: "secretish"})
 * edits are atomic so if one facet fails to succeed, entire edit fails
 */
export function editUser(username, callback, ...args) {
    // in reality, this will take (username, ...args) instead of known args and concat uri from that
    let uri = `/api/edituser?username=${encodeURIComponent(username)}`
    args.forEach((item)=>{
        uri += `&${item.name}=${encodeURIComponent(item.value)}`
    })
    fetch(uri, {
        method: 'POST',
        credentials: 'include',
    })
    .then((response) => response.json())
    .then((result) => {
        consoleLog(`API - editUser response: ${result.message}`);
        try {
            callback(result);
            /* eslint-disable no-empty */
        } catch (err) { }
    })
    .catch((error) => {
        consoleLog('API - editUser error: ');
        consoleLog(error);
        try {
            callback(error);
        } catch (err) { }
    });
}