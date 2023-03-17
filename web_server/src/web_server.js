/* eslint-disable no-console */
/* eslint-disable camelcase */
/* eslint-disable no-useless-escape */
const fs = require('fs');
const path = require('path');
const mongoose = require('mongoose');
const morgan = require('morgan');
const express = require('express');
const https = require('https');
const bodyParser = require('body-parser');
const cookieParser = require('cookie-parser');
const addRequestId = require('express-request-id')();
const expressWinston = require('express-winston');
// eslint-disable-next-line no-unused-vars
const colors = require('colors'); // this allows for addition of color to CLI,
// for example, `console.log('test'.yellow.bold)`

// eslint-disable-next-line import/no-unresolved, import/no-extraneous-dependencies
const fims = require('fims');
require('./auth/auth');
require('dotenv').config({
    // TODO: if the following does not work, use `path.resolve('../.env')`
    path: path.join(__dirname, '../.env'),
});
const expressLogger = require('./logging/expressLogger')
fims.connect('web_server');
const app = express();
const app_fleetmanager = express();

const buildPathArg = process.argv[2];
if (!buildPathArg) {
    throw new Error('ERROR: Please supply a path to built web app. Usage: sudo node web_server.js path/to/build path/to/config/ path/to/opt/'.red.bold);
}

const buildPath = path.resolve(buildPathArg, 'build');
if (!fs.existsSync(buildPath)) {
    throw new Error(`ERROR: UI build not found in ${buildPathArg}`.red.bold);
}

const buildPathFleetManager = path.resolve(buildPathArg, 'build_fleetmanager');
if (!fs.existsSync(buildPathFleetManager)) {
    console.log(`NOTE: Fleet Manager UI build not found in ${buildPathArg}, no Fleet Manager UI will be served.`.yellow.bold);
} else {
    console.log(`Fleet Manager UI build found in ${buildPathArg}, serving Fleet Manager.`.green.bold);
}

const configPathArg = process.argv[3];
if (!configPathArg) {
    throw new Error('ERROR: Please supply a path to web_ui.json file. Usage: sudo node web_server.js path/to/build path/to/config/ path/to/opt/'.red.bold);
}

const optPathArg = process.argv[4];
// any new security permissions will be uploaded from `permissions.json` at
// process.argv[4] (which is usually `/opt/web_server`). After ingestion
// below for substitution in the "hybridos_authentication" Mongo database,
// permissions.json will be deleted for security purposes.

const configPathAndFile = path.resolve(configPathArg, 'web_ui.json');
if (!fs.existsSync(configPathAndFile)) {
    throw new Error(`web_ui.json not found in ${configPathArg}`);
}

// eslint-disable-next-line import/no-mutable-exports, import/prefer-default-export
let siteConfiguration;
try {
    // eslint-disable-next-line import/no-dynamic-require, global-require
    siteConfiguration = require(`${configPathAndFile}`);
} catch (error) {
    throw new Error(`ERROR: web_ui.json was not parsed successfully: ${error}`.red.bold);
}

const serverPathAndFile = path.resolve(optPathArg, 'web_server.json');

if (!fs.existsSync(serverPathAndFile)) {
    throw new Error(`web_server.json not found in ${optPathArg}`);
}

let serverConfiguration;
try {
    // eslint-disable-next-line import/no-dynamic-require, global-require
    serverConfiguration = require(`${serverPathAndFile}`);
} catch (error) {
    throw new Error(`ERROR: web_server.json was not parsed successfully: ${error}`.red.bold);
}

const sslPath = path.resolve(configPathArg, 'ssl');
let options;
try {
    options = {
        key: fs.readFileSync(path.join(sslPath, 'hybridos-key.pem')),
        cert: fs.readFileSync(path.join(sslPath, 'hybridos-cert.pem')),
    };
} catch (error) {
    throw new Error(`SSL keys not found in ${sslPath} Error: ${error}`);
}

/**
 * Default server port
 * @type {number}
 */
const SOCKET_SERVER_PORT = 443;

/**
 * Port used for serving UI
 * @type {number}
 */
const FLEETMANAGER_UI_SERVER_PORT = 5000;
const httpsServer = https.createServer(options, app);
const httpsServer_fleetmanager = https.createServer(options, app_fleetmanager);
const io = require('socket.io')(httpsServer, { transports: ['websocket', 'polling'] });
const {
    secureRoutes,
    getRateSite,
    getRateFeatures,
    getRateAssets,
    getRateComponents,
    getRateEvents,
    getRateInspector,
    postRateSite,
    postRateFeatures,
    postRateAssets,
    postRateComponents,
    postRateEvents,
    postRateInspector,
} = require('./routes/secure-routes');
const {
    restRoutes,
    queryIDandRes,
} = require('./routes/rest-routes');
const routes = require('./routes/routes');
const logger = require('./logging/logger');
// log levels are { error: 0, warn: 1, info: 2, verbose: 3, debug: 4, silly: 5 }
// NOTE: linting wants the following line below the import of io above
const { encryptAndWritePermissions, readAndDecryptPermissions } = require('./database/database');
const {
    // eslint-disable-next-line camelcase
    getPermissionsFromWeb_serverJS, getIOFromWeb_serverJS, unrestrictedRateLimiter,
    faviconRateLimiter, loginRateLimiter, APIRateLimiter,
    restRateLimiter, inspectorRateLimiter, extractCookieFromReq, checkIfValidJWT,
    invalidateJWT, addToConnectedSockets, findAndDisconnectSocketsByJWT,
    findAndInvalidateExpiredJWT, accessPerRole, verifyJWT,
} = require('./auth/authConfig');
const { createDefaultUser } = require('./database/manageDefaultUser');

// Pull worker functions for aggregatedEndpoints
const {
    initializeAggregatedEndpoints,
    getAggregatedEndpoints,
    mergeEndpoints,
} = require('./workers/aggregatedEndpoints');

// Set up aggregatedEndpoints with empty objects
const aggregatedEndpoints = initializeAggregatedEndpoints(serverConfiguration);

getIOFromWeb_serverJS(io); // this feeds the io object to authConfig

httpsServer.listen(SOCKET_SERVER_PORT, () => {
    const theMessage = `          * * * * * * * STARTUP NOTIFICATION - web_server listening on port ${SOCKET_SERVER_PORT} * * * * * * *          `;
    logger.info(theMessage);
    logger.warn(theMessage);
    logger.error(theMessage);
});
httpsServer_fleetmanager.listen(FLEETMANAGER_UI_SERVER_PORT, () => {
    const theMessage = `          * * * * * * * STARTUP NOTIFICATION - web_server listening for Fleet Manager on port ${FLEETMANAGER_UI_SERVER_PORT} * * * * * * *          `;
    logger.info(theMessage);
    logger.warn(theMessage);
    logger.error(theMessage);
});
morgan.token('id', (req) => req.id);

app.use(function(req, res, next) {
    if(req.secure) {
        res.setHeader('Strict-Transport-Security', 'max-age=31536000; includesSubDomains')
    }
    next()
})

app.use((req, res, next) => {
    res.header('Access-Control-Allow-Origin', '*');
    res.header('Access-Control-Allow-Headers', 'Origin, X-Requested-With, Content-Type, Accept, Authorization');
    res.header('X-Content-Type-Options', 'nosniff');
    next();
});
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));
app.use(addRequestId);
app.use(express.static(buildPath));
if (buildPathFleetManager) {
    app_fleetmanager.use(express.static(buildPathFleetManager));
}
app.use(addRequestId);
app.use(expressLogger)

// rate limiter settings for every path
app.use('/', unrestrictedRateLimiter);
app.use('/favicon.ico', faviconRateLimiter);
app.use('/login', loginRateLimiter);
app.use('/api', APIRateLimiter);
app.use('/rest', restRateLimiter);
app.use('/inspector', inspectorRateLimiter);

// the following is standard Apache "combined" log output with req.id prepended to it
// app.use(morgan(':id :remote-addr - :remote-user [:date[clf]] ":method :url HTTP/:http-version"
// :status :res[content-length] ":referrer" ":user-agent')); // TODO: uncomment this?
morgan.token('id', (req) => req.id);
app.use(cookieParser(process.env.JWT_SECRET));
// works with `passport.authenticate('jwt-cookiecombo'` in secure-routes.js
app.use('/', routes);
app.use('/inspector', secureRoutes);
app.use('/api', secureRoutes);
app.use('/rest///', restRoutes);
app.use('/rest//', restRoutes);
app.use('/rest/', restRoutes);
// the three lines above allow REST API users to erroneously put up to four slashes between
// "rest" and the next segment of the URI (e.g., "/rest////features") without causing an error.
// Extra slashes after that point are handled in the /src/routes/rest-routes.js code.
app.use('/rest', restRoutes);
// the following is one way to do passport.authenticate, but I could not figure out a way to be able
// to handle authentication failures (mainly getting info to log them), so I just moved the
// passport.authenticate part into each path in secureRoutes
// app.use('/user', passport.authenticate('jwt-cookiecombo', { session: false }), secureRoutes);

// Handle errors
app.use((err, req, res) => {
    let message = "";
    try {
        res.status(err.status || 500);
        res.json({ error: err });
        if (err && err.message)
            message = `message ${err.message},`;

    } catch (error) {
        logger.error(`ERROR - could not send error from server.js. Original error: status ${err.status}, ${message} error sending: ${error}`);
    }
});

// for mongoose@6.3.5, currently unused
// mongoose.connect('mongodb://localhost:27017/hybridos_authentication')
//     .catch(error => logger.error(error));    // during init
// mongoose.connection.on('error', (error) => logger.error(error));    // after init
mongoose.connect('mongodb://localhost:27017/hybridos_authentication', { useNewUrlParser: true, useUnifiedTopology: true, useCreateIndex: true });
mongoose.connection.on('error', (error) => logger.error(error));
mongoose.Promise = global.Promise;

/**
 * Logs current user permission and access to console and create's default user
 * @param {object} result holds permission and access data
 */
function wrapUpUsersAndPermissions(result) {
    console.log('\n+++++ permissions sample returned from database. roleOrUsername:', result[0].roleOrUsername, 'access:', result[0].access[0]);
    createDefaultUser(); // creates default user only if zero users in mongodb
    console.log('- - - - - - - - - - - -');
}

// if a new permissions.json file exists in /opt/web_server (or whatever path `process.argv[4]`
// has supplied), it will be read and parsed here, then overwritten and deleted.
const optPathAndFile = path.resolve(optPathArg, 'permissions.json');
console.log('\n- - - - - - - - - - - -');
if (fs.existsSync(optPathAndFile)) {
    console.log('+++++ new permissions file recognized...\n'.yellow.bold);
    try {
        const thePermissionsTemp = JSON.parse(fs.readFileSync(optPathAndFile));
        encryptAndWritePermissions(thePermissionsTemp.authentication.permissions, (result) => {
            // the result is an array of roleOrUsername/access objects
            getPermissionsFromWeb_serverJS(result);
            // this ^ feeds the permissions array to authConfig
            wrapUpUsersAndPermissions(result);
        });
    } catch (err) {
        throw new Error(`ERROR: permissions.json was not parsed successfully: ${err}`.red.bold);
    }
} else {
    readAndDecryptPermissions((result) => {
        getPermissionsFromWeb_serverJS(result); // this feeds the permissions array to authConfig
        console.log('+++++ using existing permissions.');
        wrapUpUsersAndPermissions(result);
    });
}

const authSocket = io.of('/api');
authSocket.on('connection', (socket) => {
    // console.log('>>>>>>> authSocket connecting');
    socket.emit('siteConfiguration', { siteConfiguration });
    // TODO: only emit the access for the roleOrUsername that has logged on.
    socket.emit('accessPerRole', { accessPerRole });
    let theJWT = extractCookieFromReq(socket.handshake);
    if (checkIfValidJWT(theJWT, 'socket', socket)) {
        verifyJWT(theJWT, socket, 'connection').then((result) => {
            if (result) {
                socket.emit('message', { message: `SOCKET: JWT verified socket.io connection received: ${socket.id}` });
                // socket.emit('siteConfiguration', { siteConfiguration });
                // socket.emit('accessPerRole', { accessPerRole });
                addToConnectedSockets({ jwt: theJWT, socket: socket.id }, socket);
                if (result && result.user) {
                    socket.on('message', (data) => {
                        theJWT = extractCookieFromReq(socket.handshake);
                        if (checkIfValidJWT(theJWT, 'socket', socket)) {
                            verifyJWT(theJWT, socket, 'message').then((secondResult) => {
                                if (secondResult) {
                                    addToConnectedSockets({ jwt: theJWT, socket: socket.id },
                                        socket);
                                    // console.log(`>>>>>>> SOCKET MESSAGE: JWT verified message
                                    // from ${socket.id}: '${data.message}'`);
                                    logger.info(`WEB_SERVER SOCKET - JWT verified message from ${socket.id}: '${data.message}'`);
                                    if (data.message === 'invalidate' || data.message === 'unloading') {
                                        const theMessage = `WEB_SERVER AUTH - UNLOADING: received browser unloading notification from socket ${socket.id}`;
                                        logger.warn(theMessage);
                                        logger.info(theMessage);
                                        invalidateJWT(theJWT, null);
                                    }
                                } else {
                                    logger.warn(`WEB_SERVER SOCKET - UNAUTHORIZED message rejected: "${data.message}". Automatically disconnected unauthorized client from server. Socket ID: ${socket.id}`);
                                }
                            });
                        }
                    });
                    socket.on('disconnect', () => {
                        // the socket is dead, now we have to remove it from the socket tracker
                        findAndDisconnectSocketsByJWT(theJWT);
                    });
                } else {
                    logger.warn(`WEB_SERVER SOCKET - ERROR: message not authorized. referer: ${socket.handshake.headers.referer}, user-agent: ${socket.handshake.headers['user-agent']}`);
                }
            } else {
                logger.warn(`WEB_SERVER SOCKET - ERROR: connection not authorized. referer: ${socket.handshake.headers.referer},user-agent: ${socket.handshake.headers['user-agent']}`);
            }
        });
    }
});

// the following lines make a connection for socket.io messaging between web_ui and web_server
let emitCompleteBodies = false;
const inspectorControl = io.of('/inspectorControl');
inspectorControl.on('connection', (socket) => {
    // console.log('>>>>>>> inspectorControl connecting');
    let theJWT = extractCookieFromReq(socket.handshake);
    if (checkIfValidJWT(theJWT, 'socket', socket)) {
        verifyJWT(theJWT, socket, 'connection').then((result) => {
            if (result) {
                // console.log('>>>>>>> user role is valid for inspectorControl');
                socket.emit('message', { message: 'SOCKET: JWT verified inspectorControl connection received' });
                addToConnectedSockets({ jwt: theJWT, socket: socket.id }, socket);
                if (result && result.user) {
                    socket.on('message', (data) => {
                        theJWT = extractCookieFromReq(socket.handshake);
                        if (checkIfValidJWT(theJWT, 'socket', socket)) {
                            verifyJWT(theJWT, socket, 'message').then((secondResult) => {
                                if (secondResult) {
                                    addToConnectedSockets({ jwt: theJWT, socket: socket.id },
                                        socket);
                                    // console.log(`>>>>>>> SOCKET MESSAGE: JWT verified message
                                    // from ${socket.id}: '${data.message}'`);
                                    logger.info(`WEB_SERVER SOCKET - JWT verified message from ${socket.id}: '${data.message}'`);
                                    if (data.sender === 'SingleFIMS') {
                                        switch (data.message) {
                                            case 'start listening':
                                                emitCompleteBodies = true;
                                                break;
                                            case 'stop listening':
                                                emitCompleteBodies = false;
                                                fims.unsubscribeFrom('/components');
                                                break;
                                            case 'open components':
                                                fims.subscribeTo('/components');
                                                break;
                                            default:
                                            // code block
                                        }
                                    }
                                }
                            });
                        }
                    });
                }
            }
        });
    }
});
// the above lines make a connection for messaging

// these sockets never receive messages so we don't do any handling
const site = io.of('/site');
const features = io.of('/features');
const assets = io.of('/assets');
const events = io.of('/events');
const metrics = io.of('/metrics');
const sites = io.of('/sites');
const components = io.of('/components');
const inspector = io.of('/inspector');
const heart1000Emit = io.of('/heart1000');
const serverCountEmit = io.of('/serverCount');
const scheduler = io.of('/scheduler');

// the following variables are for tracking pubs and sets per second
// this info can be viewed in "Throughput Display" in web_ui
let pubRateSite = 0;
let pubRateFeatures = 0;
let pubRateAssets = 0;
let pubRateComponents = 0;
let pubRateInspector = 0;
let setRateSite = 0;
let setRateFeatures = 0;
let setRateAssets = 0;
let setRateComponents = 0;
let setRateEvents = 0;
let setRateInspector = 0;
let theCountData = null;
let theListenForMessagesCount = 0;
let theReceivedMessagesCount = 0;
let theMaxReceivedMessagesCount = 0;

function countMessages(count) {
    theCountData = {pubRateSite, pubRateFeatures, pubRateAssets, pubRateComponents, pubRateInspector,
    setRateSite, setRateFeatures, setRateAssets, setRateComponents, setRateInspector, setRateEvents,
    getRateSite, getRateFeatures, getRateAssets, getRateComponents, getRateInspector, getRateEvents,
    postRateSite, postRateFeatures, postRateAssets, postRateComponents, postRateInspector, postRateEvents,
    theListenForMessagesCount, theReceivedMessagesCount, theMaxReceivedMessagesCount, count};

    serverCountEmit.emit('/serverCount', (theCountData));
    pubRateSite = 0;
    pubRateFeatures = 0;
    pubRateAssets = 0;
    pubRateComponents = 0;
    pubRateInspector = 0;
    setRateSite = 0;
    setRateFeatures = 0;
    setRateAssets = 0;
    setRateComponents = 0;
    setRateEvents = 0;
    setRateInspector = 0;
    theListenForMessagesCount = 0;
    if (theMaxReceivedMessagesCount < theReceivedMessagesCount) {
        theMaxReceivedMessagesCount = theReceivedMessagesCount;
    }
    theReceivedMessagesCount = 0;
}

function sendHeartbeatMsg(count) {
    const heartbeatMsg = {
        method: 'pub',
        uri: '/heart1000',
        replyto: null,
        body: {},
    };
    heartbeatMsg.body.count = count;
    heartbeatMsg.body.timestamp = Date.now();
    // we'll send the heartbeat on FIMS (optional) and as an emit to web_ui
    // fimsApi.send(heartbeatMsg);
    heart1000Emit.emit('/heart1000', JSON.stringify(heartbeatMsg.body));
}

const heart1000 = {};
/**
 * Heartbeat function for running time
 * @param {function} func recursive callback for heartbeat 
 * @param {number} delay time between heartbeats 
 */
function beat(func, delay) {
    if (!heart1000.started) {
        /* eslint-disable no-param-reassign */
        heart1000.func = func;
        heart1000.delay = delay;
        heart1000.startTime = Date.now();
        heart1000.target = delay;
        heart1000.started = true;
        heart1000.count = 1;
        setTimeout(beat, delay);
    } else {
        const elapsed = Date.now() - heart1000.startTime;
        const adjust = heart1000.target - elapsed;
        heart1000.count += 1;
        heart1000.func(heart1000.count);
        heart1000.target += heart1000.delay;
        setTimeout(beat, heart1000.delay + adjust);
        /* eslint-enable no-param-reassign */
    }
}

// Helper function for testing side effects in aggregatedEndpoints
function fimsSend(msg) {
    fims.send(msg);
}

beat((count) => {
    countMessages(count);
    sendHeartbeatMsg(count);
    // Generate and send FIMS 'get' messages to retrieve endpoint data
    getAggregatedEndpoints(aggregatedEndpoints, fimsSend);
}, 1000);

// finds and deletes expired JSON Web Tokens
setInterval(() => {
    findAndInvalidateExpiredJWT();
}, 300000)

// NOTE: this method of server control from web_ui can be inefficient because it
// is asynchronous. It is not good for quick switching of server functions on and
// off, so we use the same functionality through socket.io near the top of this file
// app.post('/api/server/:sender/:message', (req, res) => {
//     if (req.params.sender === 'SingleFIMS') {
//         switch (req.params.message) {
//             case 'startListening':
//                 emitCompleteBodies = true;
//                 res.status(200).send('{}');
//                 break;
//             case 'stopListening':
//                 emitCompleteBodies = false;
//                 fims.unsubscribeFrom('/components');
//                 res.status(200).send('{}');
//                 break;
//             case 'openComponents':
//                 fims.subscribeTo('/components');
//                 res.status(200).send('{}');
//                 break;
//             default:
//             // code block
//         }
//     }
// });

/**
 * List of URIs to subscribe to (node fims must subscribe in order to listen to a URI)
 * @type {string[]}
 */
const theSocketListeners = [
    '/site',
    '/ui/site',
    '/features',
    '/ui/features',
    '/assets',
    '/ui/assets',
    '/inspectorControl',
    '/ui/inspectorControl',
    '/inspector',
    '/ui/inspector',
    '/scheduler',
    '/ui/scheduler',
    '/ui/events',
    '/rest',
    '/metrics',
    '/sites',
    '/aggregate'
];
if (siteConfiguration.met_station) {
    const theURIs = ['/components/met_station', '/ui/components/met_station'];
    theURIs.forEach((uri) => theSocketListeners.push(uri));
}
if (siteConfiguration.tracker) {
    const theURIs = ['/components/tracker_summary', '/ui/components/tracker_summary'];
    theURIs.forEach((uri) => theSocketListeners.push(uri));
}
if (siteConfiguration.inspectorComponentsName) {
    theSocketListeners.push(`/components/${siteConfiguration.inspectorComponentsName}`);
    theSocketListeners.push(`/ui/components/${siteConfiguration.inspectorComponentsName}`);
}

/**
 * Subscribes fims to list of URIs
 */
function bindSocketListeners() {
    fims.subscribeToList(theSocketListeners);
}

/**
 * Receives fims messages and processes them
 */
function listenForMessage() {
    theListenForMessagesCount += 1;
    fims.receiveWithTimeout(50, (data) => { // microseconds
        if (data) {
            theReceivedMessagesCount += 1;
            const pathFirstPart = data.uri.split('/')[1];
            const uriLastIndex = data.uri.lastIndexOf('/');
            if (pathFirstPart === 'rest') {               
                // if the message is a response to a REST API call then we
                // use its unique ID to look up the res to send the data
                // back to
                const theDataURISplit = data.uri.split('/');
                const theQueryID = theDataURISplit[theDataURISplit.length - 1];
                queryIDandRes.forEach((theID, i) => {
                    if (theID.queryID === parseInt(theQueryID, 10)) {
                        queryIDandRes.splice(i - 1, 1);
                        if (!theID.res.headersSent) {
                            if (typeof data.body === 'object') {
                                if (data.uri.includes('/dbi/')) {
                                    theID.res.status(200).json(data);
                                } else {
                                    theID.res.status(200).send(data.body);
                                }
                            } else {
                                // it is just a naked value
                                // example of a control point replying with a naked body:
                                /*
                                { method: 'set',
                                  uri: '/rest/components/alvin/gen_basepoint/1601322861205',
                                  replyto: 0,
                                  body: 0 }
                                */
                                const theBody = {};
                                theBody.uri = data.uri.substr(0, uriLastIndex);
                                theBody.value = data.body;
                                theID.res.status(200).send(theBody);
                            }
                        }
                    }
                });
            } else if (pathFirstPart === 'aggregate') {
                // Receive FIMS gets on /aggregate to merge into aggregatedEndpoints
                // Not a deep merge, if issues then switch to lodash.merge
                mergeEndpoints(aggregatedEndpoints, data.uri.replace('/aggregate', ''), data.body);
            } else {
                // the vast majority of FIMS messages are not REST API calls
                // and are processed here
                const dataBody = emitCompleteBodies ? JSON.stringify(data)
                    : JSON.stringify(data.body);
                const dataURI = emitCompleteBodies ? 'message' : data.uri;
                if (data.method === 'pub') {
                    // Check if FIMS pub is in aggregatedEndpoints, then aggregate
                    if (aggregatedEndpoints[data.uri.slice(0, uriLastIndex)]) {
                        // Not a deep merge, if issues then switch to lodash.merge
                        mergeEndpoints(aggregatedEndpoints, data.uri, data.body);
                    }
                    // we only ever listen to pubs sent to /site, etc., without /ui in the path.
                    switch (pathFirstPart) {
                        case 'site':
                            pubRateSite += 1;
                            site.emit(dataURI, dataBody);
                            break;
                        case 'features':
                            pubRateFeatures += 1;
                            features.emit(dataURI, dataBody);
                            break;
                        case 'assets':
                            pubRateAssets += 1;
                            assets.emit(dataURI, dataBody);
                            break;
                        case 'events':
                            // pubRateComponents += 1;
                            events.emit(dataURI, dataBody);
                            break;
                        case 'metrics':
                            // pubRateComponents += 1;
                            metrics.emit(dataURI, dataBody);
                            break;
                        case 'sites':
                            // pubRateComponents += 1;
                            sites.emit(dataURI, dataBody);
                            break;
                        case 'components':
                            pubRateComponents += 1;
                            components.emit(dataURI, dataBody);
                            break;
                        case 'inspector':
                            pubRateInspector += 1;
                            inspector.emit(dataURI, dataBody);
                            break;
                        case 'inspectorControl':
                            pubRateInspector += 1;
                            inspectorControl.emit(dataURI, dataBody);
                            break;
                        case 'scheduler':
                            scheduler.emit(dataURI, dataBody);
                            break;
                        default:
                            break;
                    }
                }
                if (data.method === 'set' && pathFirstPart === 'ui') {
                    // a 'get' is returned to ui as a 'set'
                    // the only time we 'get' is on loading a new page
                    const emitUri = dataURI.replace('/ui', '');
                    switch (data.uri.split('/')[2]) {
                        case 'site':
                            setRateSite += 1;
                            site.emit(emitUri, dataBody);
                            break;
                        case 'features':
                            setRateFeatures += 1;
                            features.emit(emitUri, dataBody);
                            break;
                        case 'assets':
                            setRateAssets += 1;
                            assets.emit(emitUri, dataBody);
                            break;
                        case 'events':
                            setRateEvents += 1;
                            events.emit(emitUri, dataBody);
                            break;
                        case 'metrics':
                            // setRateEvents += 1;
                            metrics.emit(emitUri, dataBody);
                            break;
                        case 'sites':
                            // setRateEvents += 1;
                            sites.emit(emitUri, dataBody);
                            break;
                        case 'components':
                            setRateComponents += 1;
                            components.emit(emitUri, dataBody);
                            break;
                        case 'inspector':
                            setRateInspector += 1;
                            inspector.emit(emitUri, dataBody);
                            break;
                        case 'inspectorControl':
                            setRateInspector += 1;
                            inspectorControl.emit(emitUri, dataBody);
                            break;
                        case 'scheduler':
                            scheduler.emit(emitUri, dataBody);
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    });
    setTimeout(() => {
        listenForMessage();
    }, 2);
}

bindSocketListeners();
listenForMessage();

exports.io = io;
exports.systemInformationRequests = serverConfiguration.systemInformationRequests;
exports.aggregatedEndpoints = aggregatedEndpoints;
