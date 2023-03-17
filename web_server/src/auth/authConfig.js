/* eslint-disable no-console */
// this file holds authorization functions, options, and data arrays for tracking.
// rate limiter options are also kept here.

const jwt = require('jsonwebtoken');
const rateLimit = require('express-rate-limit');
const logger = require('../logging/logger');

const validJWTs = [];
const connectedSockets = [];
// the following numbers give us the start and end of a substring of characters
// in a JWT that are easily differentiated from one JWT to another. There are
// many ranges of characters that are different in each JWT, but this range stood
// out to me and appears to be reliable for easy human differentiation. -DM
const shortJWTIdentifierStartCharacter = 306;
const shortJWTIdentifierEndCharacter = 321;

const accessPerRole = {};
// eslint-disable-next-line camelcase
/**
 * Retrieves permissions from web_server.js
 * @param {object} permissions permissions object
 */
function getPermissionsFromWeb_serverJS(permissions) {
    for (let i = 0; i < permissions.length; i += 1) {
        accessPerRole[permissions[i].roleOrUsername] = permissions[i].access;
    }
}

let io;
// eslint-disable-next-line camelcase
/**
 * Connects io to authConfig
 * @param {object} ioObject io object
 */
function getIOFromWeb_serverJS(ioObject) {
    io = ioObject;
}

/**
 * Gets JWT cookie from request object
 * @param {object} req request object
 * @returns {string} JWT if available
 */
function extractCookieFromReq(req) {
    let theJWT;
    if (req.headers.cookie && (req.headers.cookie.includes(';') || req.headers.cookie.includes('jwt=s%3A'))) {
        // extracts JWT from secure cookie in req from browser.
        // cookies can be in any order, the JWT is probably not
        // the only cookie.
        const theCookies = req.headers.cookie.split(/; */);
        const theJWTCookie = theCookies.find((cookie) => cookie.includes('jwt=s%3A'));

        if (theJWTCookie && theJWTCookie.length > 0) {
            theJWT = theJWTCookie.split('jwt=s%3A')[1].split('.'); // removes
            // unnecessary characters from the front of the jwt cookie
            // and then splits it by the periods in the jwt
            theJWT.pop(); // removes the last element (the characters
            // after the last period)
            theJWT = theJWT.join('.').toString(); // rejoins the pieces
        } else {
            theJWT = 'no_JWT_found';
        }
    } else {
        theJWT = 'no_JWT_found';
    }
    return theJWT;
}

/**
 * Checks for valid JWTs
 * @param {string} jwtToCheck 
 * @param {string} requestType socket or other api
 * @param {object} req request object
 * @param {object} res response object
 * @returns {boolean} true if valid JWT
 */
function checkIfValidJWT(jwtToCheck, requestType, req, res) {
    // check if a JWT is in our array of valid JWTs. JWTs carry expiration
    // dates, but we also need to be able to invalidate them at any time.
    // we only display a substring of the JWT with obviously-different characters.
    const theRequestorString = requestType === 'socket' ? ` ${req.id.replace('/api#', '')}` : '';
    const theJWTString = jwtToCheck.substring(shortJWTIdentifierStartCharacter, shortJWTIdentifierEndCharacter) ? jwtToCheck.substring(shortJWTIdentifierStartCharacter, shortJWTIdentifierEndCharacter) : '(malformed JWT)';
    const index = validJWTs.findIndex((theJWT) => theJWT === jwtToCheck);
    if (index === -1) {
        try {
            logger.warn(`BROWSER AUTH - CHECK JWT VALIDITY for ${requestType}${theRequestorString}: JWT ${theJWTString} is INVALID. REQUESTOR SUPPLIED INVALID JWT. IF we have not just invalidated this JWT for other reasons, then this may indicate an intruder is attempting access. request: ${req.method} ${req.originalUrl}, host: ${requestType === 'socket' ? req.handshake.headers.host : req.headers.host}, user-agent: ${requestType === 'socket' ? req.handshake.headers['user-agent'] || null : req.headers['user-agent'] || null}`);
            if (jwtToCheck.substring(shortJWTIdentifierStartCharacter,
                shortJWTIdentifierEndCharacter)) {
                // if the substring isn't there then it is a malformed JWT so trying to
                // invalidate won't do anything
                // eslint-disable-next-line no-use-before-define
                invalidateJWT(jwtToCheck, res);
            } else if (res) {
                res.sendStatus(401);
                logger.warn('BROWSER AUTH - INVALIDATING: "Unauthorized" http status sent.');
            }
        } catch (error) {
            console.log('/web_server/src/auth/authConfig/checkIfValidJWT error:', error);
        }
        return false;
    }
    logger.info(`BROWSER AUTH - CHECK JWT VALIDITY for ${requestType}${theRequestorString}: JWT ${theJWTString} is valid`);
    return true;
}

/**
 * Adds socket to list of current connections
 * @param {object} clientObject client connection object
 * @param {object} socket socket connection object
 */
function addToConnectedSockets(clientObject, socket) {
    // this function prevents duplicates in the connectedSockets array.
    // by comparison, there cannot be duplicates in the validJWTs array
    // because it gets items added to it only at the moment the JWT is
    // signed. Connected sockets pop up somewhat randomly; we keep track
    // of them for later disconnection when a JWT expires or becomes
    // invalidated.
    const index = connectedSockets.findIndex((client) => client.socket === clientObject.socket);
    if (index === -1) {
        connectedSockets.push(clientObject);
        logger.info(`BROWSER SOCKET - ADDED: JWT verified socket.io connection added: ${clientObject.socket}, host: ${socket.handshake.headers.host}, user-agent: ${socket.handshake.headers['user-agent']}`);
    } else {
        logger.info(`BROWSER SOCKET - TRACKED: JWT verified socket.io connection already tracked: ${clientObject.socket}, host: ${socket.handshake.headers.host}, user-agent: ${socket.handshake.headers['user-agent']}`);
    }
}

/**
 * Removes socket from list of active sockets
 * @param {string} socketID id of socket
 */
function disconnectSocket(socketID) {
    // eslint-disable-next-line no-param-reassign
    socketID = socketID.replace('/api#', '');
    try {
        io.sockets.sockets[socketID].emit('unauthorized', { message: 'SOCKET: disconnecting socket' });
        io.sockets.sockets[socketID].disconnect();
    } catch (err) {
        logger.info('BROWSER SOCKET - emit and/or disconnect to socket failed. This usually means that the socket was already disconnected.');
    }
}

/**
 * Removes all sockets connected to expired or invalidated JWT
 * @param {string} jwtToDisconnect JWT to disconnect sockets
 */
function findAndDisconnectSocketsByJWT(jwtToDisconnect) {
    // disconnects all sockets related to an expired or invalidated JWT
    logger.info(`BROWSER SOCKET - DISCONNECTING BY JWT: JWT ${jwtToDisconnect.substring(shortJWTIdentifierStartCharacter, shortJWTIdentifierEndCharacter)}`);
    const updatedConnectedSockets = [];
    connectedSockets.forEach((client, i) => {
        if (client.jwt === jwtToDisconnect) {
            logger.info(`BROWSER SOCKET - DISCONNECTING BY JWT: disconnecting socket ${client.socket} at index ${i} of connectedSockets, associated with jwt: ${client.jwt.substring(shortJWTIdentifierStartCharacter, shortJWTIdentifierEndCharacter)}`);
            disconnectSocket(client.socket);
        } else {
            updatedConnectedSockets.push(client);
        }
    });
    // the following replacement of the connectedSockets array can
    // be done differently, depending on your school of thought. In
    // this shop, at this time, we will follow the school of thought
    // that arrays are always declared as "const" unless there is a
    // reason they *cannot* be. In that case, this is a correct way
    // to replace the contents of this array. If connectedSockets
    // had been declared as a "let" then we could write:
    // connectedSockets = updatedConnectedSockets;
    // Clearly this would be a shorter and simpler way to do it,
    // but it would be inconsistent with every other way we interact
    // with arrays. -DM 033020
    connectedSockets.splice(0, connectedSockets.length, ...updatedConnectedSockets);
}

/**
 * Removes JWT from validJWTs array and disconnects all connections
 * @param {string} jwtToInvalidate JWT to invalidate
 * @param {object} res response object
 */
function invalidateJWT(jwtToInvalidate, res) {
    if (validJWTs.length > 0) {
        // remove a JWT from the validJWTs array, send unauthorized status (401) if we have a res,
        // and disconnect all sockets associated with this JWT
        const indexToInvalidate = validJWTs.findIndex((theJWT) => theJWT === jwtToInvalidate);
        if (indexToInvalidate === -1) {
            logger.info(`BROWSER AUTH - INVALIDATING: JWT ${jwtToInvalidate.substring(shortJWTIdentifierStartCharacter, shortJWTIdentifierEndCharacter)} is not in valid JWTs list`);
        } else {
            logger.info(`BROWSER AUTH - INVALIDATING: JWT ${jwtToInvalidate.substring(shortJWTIdentifierStartCharacter, shortJWTIdentifierEndCharacter)} exists, INVALIDATING now...`);
            validJWTs.splice(indexToInvalidate, 1);
        }
        if (res) {
            res.sendStatus(401);
            logger.info('BROWSER AUTH - INVALIDATING: "Unauthorized" http status sent.');
        }
        findAndDisconnectSocketsByJWT(jwtToInvalidate);
    } else {
        logger.info(`BROWSER AUTH - INVALIDATING: JWT ${jwtToInvalidate.substring(shortJWTIdentifierStartCharacter, shortJWTIdentifierEndCharacter)}`);
    }
}

/**
 * Invalidates expired JWTs
 */
function findAndInvalidateExpiredJWT() {
    if (validJWTs.length > 0) {
        validJWTs.forEach(theJWT => {
            jwt.verify(theJWT, process.env.JWT_SECRET, (err) => {
                if (err) {
                    if (err.name === "TokenExpiredError") invalidateJWT(theJWT);
                }
            })
        })
    }
}

/**
 * Compares userAgent in JWT with the user-agent in the req object
 * @param {object} req request object
 * @param {object} res response object
 * @param {*} user unused
 * @param {*} userAgent user agent
 * @returns {boolean} true if user agent is a match
 */
function compareUserAgentInJWT(req, res, user, userAgent) {
    // compares the userAgent stored in the JWT with the user-agent
    // that is supplied in the req. If the user agents don't match,
    // we invalidate and disconnect the JWT
    if (userAgent !== req.headers['user-agent']) {
        logger.warn(`BROWSER AUTH - ALERT: user agent of request does not match JWT. request user agent: ${req.headers['user-agent']}, JWT user agent: ${userAgent}`);
        invalidateJWT(extractCookieFromReq(req), res);
        return false;
    }
    return true;
}

/**
 * Checks if user has permissions to access URI, otherwise disconnects and invalidate's JWT
 * @param {string} permission: permission is the access allowed for the endpoint itself
 * @param {object} data: userName-str, role-str, accessPoint-str, req-obj, res-obj, socketID-str, theJWT-str
 * @returns {boolean} true if access is permitted
 */
function confirmRoleAccess(permission, data) {
    // eslint-disable-next-line no-param-reassign
    let accessPoint;
    if (data.accessPoint.includes('/sites/') || data.accessPoint.includes('/components/')) {
        const uriBase = data.accessPoint.includes('/sites/') ? '/sites' : '/components';
        accessPoint = `${uriBase}/any_site${data.accessPoint.split('/').length > 3 ? `/${data.accessPoint.split('/').slice(3, data.accessPoint.length).join('/')}` : ''}`;
    } else if (data.accessPoint.includes('/dbi/')) {
        accessPoint = '/dbi';
    } else {
        accessPoint = data.accessPoint;
    }
    const access1 = (accessPerRole[data.userName] && accessPerRole[data.userName].includes(`${accessPoint} ${permission}`) > -1)
    const access2 = (accessPerRole[data.role].includes(`${accessPoint} ${permission}`) > -1)

    if (access1 || access2) {

        return true;

    } else {
        const message = `BROWSER AUTH - roleAccess INVALID, username:\\
         ${data.userName}, role: ${data.role}, accessPoint: ${accessPoint}`;
        logger.info(message);
        logger.warn(message);
        // a socket connection may not have gotten into the connectSockets, clean up
        if (data.socketID) {
            disconnectSocket(data.socketID);
        }
        if (data.theJWT) {
            invalidateJWT(data.theJWT, data.res);
        } else {
            invalidateJWT(extractCookieFromReq(data.req), data.res);
        }
        return false;
    }
}

/**
 * Verifies JWT
 * @param {string} theJWT JWT to verify
 * @param {object} socket socket to confirm
 * @param {string} accessPoint parent endpoint
 */
async function verifyJWT(theJWT, socket, accessPoint) {
    // this function is the socket version of "passport.authenticate"
    // verifies that the JWT supplied is legitimate
    // then sends it to compareUserAgentInJWT and confirmRoleAccess

    const response = await jwt.verify(theJWT, process.env.JWT_SECRET, (err, decoded) => decoded);

    const roleAccessPayload = {
        userName: response.user,
        role: response.role,
        accessPoint: accessPoint,
        socketID: socket.id,
        theJWT: theJWT
    };

    if (compareUserAgentInJWT(socket.handshake, null, response.user, response.userAgent)
        && (confirmRoleAccess("readWrite", roleAccessPayload) || confirmRoleAccess("read", roleAccessPayload))) {
        return response;
    }
    // or make result null for "io.on('connection')" in server.js
    return null;
}

/**
 * Limits request rate with params
 * @type {object}
 */
const rateLimiterOptions = {
    windowMs: 1 * 60 * 1000, // window of time in milliseconds (first number equals minutes)
    max: 10, // requests per window of time from one IP address
    message: 'request rate limit reached',
    headers: true, // puts the number of connections tried and remaining in headers
    handler(req, res) { // fires when the rate limit is exceeded
        console.log(this.message);
        logger.warn(this.message);
        invalidateJWT(extractCookieFromReq(req), res);
    },
};

console.log('>>>>>>> rate limiter options');
const unrestrictedRateLimiterOptions = { ...rateLimiterOptions };
unrestrictedRateLimiterOptions.max = 210;
unrestrictedRateLimiterOptions.message = 'unrestricted request rate limit reached';
const unrestrictedRateLimiter = rateLimit(unrestrictedRateLimiterOptions);
console.log(unrestrictedRateLimiterOptions);

const faviconRateLimiterOptions = { ...rateLimiterOptions };
faviconRateLimiterOptions.max = 30;
faviconRateLimiterOptions.message = 'favicon request rate limit reached';
const faviconRateLimiter = rateLimit(faviconRateLimiterOptions);
console.log(faviconRateLimiterOptions);

const loginRateLimiterOptions = { ...rateLimiterOptions };
loginRateLimiterOptions.max = 5;
loginRateLimiterOptions.message = 'login request rate limit reached';
const loginRateLimiter = rateLimit(loginRateLimiterOptions);
console.log(loginRateLimiterOptions);

const APIRateLimiterOptions = { ...rateLimiterOptions };
APIRateLimiterOptions.max = 210;
APIRateLimiterOptions.message = 'API request rate limit reached';
const APIRateLimiter = rateLimit(APIRateLimiterOptions);
console.log(APIRateLimiterOptions);

const restRateLimiterOptions = { ...rateLimiterOptions };
restRateLimiterOptions.max = 620;
restRateLimiterOptions.message = 'rate request rate limit reached';
const restRateLimiter = rateLimit(restRateLimiterOptions);
console.log(restRateLimiterOptions);

const inspectorRateLimiterOptions = { ...rateLimiterOptions };
inspectorRateLimiterOptions.max = 210; // cannot be more than unrestrictedRateLimiterOptions.max
inspectorRateLimiterOptions.message = 'inspector request rate limit reached';
const inspectorRateLimiter = rateLimit(inspectorRateLimiterOptions);
console.log(inspectorRateLimiterOptions);
console.log('<<<<<<<<');

module.exports = {
    getPermissionsFromWeb_serverJS,
    getIOFromWeb_serverJS,
    faviconRateLimiter,
    loginRateLimiter,
    unrestrictedRateLimiter,
    APIRateLimiter,
    restRateLimiter,
    inspectorRateLimiter,
    jwtOptions: {
        user: {
            httpOnly: true,
            sameSite: true,
            signed: true,
            secure: true
        },
        observer: {
            httpOnly: true,
            sameSite: true,
            signed: true,
            secure: true
        },
        rest: {
            httpOnly: true,
            sameSite: true,
            signed: true,
            secure: true
        },
        admin: {
            maxAge: 2592000000,
            httpOnly: true,
            sameSite: true,
            signed: true,
            secure: true
        },
        developer: {
            maxAge: 2592000000,
            httpOnly: true,
            sameSite: true,
            signed: true,
            secure: true
        }
    },
    signOptions: {
        rest: { expTime: '8h' },
        user: { expTime: '8h' },
        developer: { expTime: '30 days' },
        admin: { expTime: '30 days' },
        observer: { expTime: '8h' }
    },
    validJWTs,
    connectedSockets,
    shortJWTIdentifierStartCharacter,
    shortJWTIdentifierEndCharacter,
    extractCookieFromReq,
    checkIfValidJWT,
    invalidateJWT,
    addToConnectedSockets,
    findAndDisconnectSocketsByJWT,
    findAndInvalidateExpiredJWT,
    compareUserAgentInJWT,
    accessPerRole,
    confirmRoleAccess,
    verifyJWT,
};
