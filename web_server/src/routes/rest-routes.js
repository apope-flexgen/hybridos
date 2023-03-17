/* eslint-disable no-console */
/* eslint-disable no-useless-escape */
// use this within any .get or .put block to see how many requests we've made
// and how many are left:
// console.log(req.rateLimit);
//    or
// logger.info(req.rateLimit);

const express = require('express');
const passport = require('passport');
/* eslint-disable import/no-extraneous-dependencies, import/no-unresolved */
const fimsApi = require('fims');
// const fims = require('fims');
// we have not yet confirmed if "fims" and "fimsApi" are the same thing.
/* eslint-enable import/no-extraneous-dependencies, import/no-unresolved */
const { accessPerRole, confirmRoleAccess } = require('../auth/authConfig');
const logger = require('../logging/logger');
const webServer = require('../web_server');

fimsApi.connect('web_server');
const router = express.Router();

const queryIDandRes = [];

const { filterAndReduceRequestedObjects } = require('../workers/aggregatedEndpoints');

/**
 * Filters queries since requests are done through fims
 * @param {object} query query to check
 * @returns {boolean} true if query should be kept
 */
function sendTimeoutMessagesAndClearExpiredQueries(query) {
    if (query.timeout < Date.now()) {
        if (!query.res.headersSent) query.res.status(408).send('{}'); // send "request timeout"
    }
    return query.timeout > Date.now(); // return the ones we'll keep
}

/**
 * Filters out queries that have expired every 15 seconds
 */
function processTimeoutsInQueryIDs() {
    const tempArray = queryIDandRes.filter(sendTimeoutMessagesAndClearExpiredQueries);
    queryIDandRes.splice(0, queryIDandRes.length, ...tempArray); // replace contents
    // of queryIDandRes with the tempArray
    setTimeout(() => {
        processTimeoutsInQueryIDs();
    }, 15000); // every 15 seconds, so maximum wait for timeout would be 1 min, 15 sec
}

/**
 * Logs authentication checks
 * @param {string} userName username
 * @param {string} role role
 * @param {string} method get or set etc
 * @param {string} accessPoint parent endpoint
 * @param {*} host host
 * @param {boolean} validOrNot has access or not
 * @param {*} value value
 */
function logAuthentication(userName, role, method, accessPoint, host, validOrNot, value) {
    if (validOrNot) {
        const theMessage = `REST AUTH - roleAccess ${validOrNot}, username: ${userName}, role: ${role}, method: ${method}, accessPoint: ${accessPoint}, host: ${host}${value ? `, value: ${value}` : ''}`;
        logger.info(theMessage);
        if (validOrNot === 'INVALID') {
            logger.warn(theMessage);
        }
    } else {
        logger.info(`REST AUTH - authentication username: ${userName}, role: ${role}, method: ${method}, accessPoint: ${accessPoint}, host: ${host}${value ? `, value: ${value}` : ''}`);
    }
}

/**
 * Notifies that user is not authorized to access endpoint
 * @param {object} req request object
 * @param {object} res response object
 * @param {string} userName username
 * @param {string} role role
 * @param {string} accessPoint parent endpoint
 * @param {*} info info
 */
function replyWithUnauthorized(req, res, userName, role, accessPoint, info) {
    // this is a little different than the one in secure-routes
    // because we're not dealing with JWTs
    logger.warn(`REST AUTH - authentication ERROR: ${info}, username: ${userName}, role: ${role}, accessPoint: ${accessPoint}${info ? ` ${info}` : ''}`);
    // logger.info(`REST AUTH - authentication ERROR: ${info}. See warnLog for more information.`);
    res.json({
        status: 401,
        message: 'REST: unauthorized',
    });
    return null;
}

// helper function for authentication
/**
 * Checks authentication, used for testing as well
 * @param {string} userName username
 * @param {string} role role
 * @param {string} accessPoint parent endpoint
 * @returns {boolean} true if has permissions
 */

/**
 * Sends GET through fims
 * @param {object} req request object
 * @param {object} res response object
 * @param {string} userName username
 * @param {string} role role
 * @param {string} accessPoint parent endpoint
 */
function startTheGet(req, res, userName, role, accessPoint) {
    logAuthentication(userName, role, 'GET', accessPoint, req.headers.host);

    const roleAccessPayload = {userName, role, accessPoint};

    if (confirmRoleAccess("read", roleAccessPayload) || confirmRoleAccess("readWrite", roleAccessPayload)) {
        logAuthentication(userName, role, 'GET', accessPoint, req.headers.host, 'valid');
        if (webServer.aggregatedEndpoints[accessPoint]) {
            res.status(200).send(
                filterAndReduceRequestedObjects(
                    webServer.aggregatedEndpoints,
                    accessPoint,
                    userName,
                    role,
                    confirmRoleAccess,
                ),
            );
        } else {
            const queryID = Date.now() + Math.floor(100000 + Math.random() * 900000);
            queryIDandRes.push({ queryID, res, timeout: Date.now() + 60000 });
            // set timeout for 60 seconds from now
            const msg = {
                method: 'get',
                uri: req.url,
                replyto: `/rest${req.url}/${queryID}`,
                body: null,
                username: userName
            };
            fimsApi.send(msg);
        }
    } else {
        logAuthentication(userName, role, 'GET', accessPoint, req.headers.host, 'INVALID');
        replyWithUnauthorized(req, res, userName, role, accessPoint, 'unauthorized rr04');
    }
}

/**
 * Sends PUT through fims
 * @param {object} req request object
 * @param {object} res response object
 * @param {string} userName username
 * @param {string} role role
 * @param {string} accessPoint parent endpoint
 * @param {*} theValue value to put
 */
function startThePut(req, res, userName, role, accessPoint, theValue) {
    logAuthentication(userName, role, 'PUT', accessPoint, req.headers.host, null, theValue);

    const roleAccessPayload = {userName, role, accessPoint};

    if (confirmRoleAccess("readWrite", roleAccessPayload)){
        logAuthentication(userName, role, 'PUT', accessPoint, req.headers.host, 'valid', theValue);
        let theValueCoerced;
        if (typeof theValue === 'string' && (theValue === 'true' || theValue === 'false')) {
            theValueCoerced = theValue !== 'false';
        } else {
            theValueCoerced = theValue;
        }
        
        const msg = {
            method: 'set',
            uri: accessPoint,
            replyto: null,
            body: `{"value":${theValueCoerced}}`,
            username: userName
        };
        fimsApi.send(msg);
        const result = {
            status: 202,
            statusString: 'accepted',
            method: 'PUT',
            uri: accessPoint,
            value: theValueCoerced,
        };
        res.status(202).send(result);
    } else {
        logAuthentication(userName, role, 'PUT', accessPoint, req.headers.host, 'INVALID', theValue);
        replyWithUnauthorized(req, res, userName, role, accessPoint, 'unauthorized rr04');
    }
}

// eslint-disable-next-line consistent-return
/**
 * Checks if URI is valid through regex
 * @param {object} req request object
 * @param {object} res response object
 * @param {*} theMessage message to send
 * @returns {boolean} true if valid URI
 */
function checkThatRequestIsClean(req, res, theMessage) {
    if ((req.url.length > 1000) || (decodeURIComponent(req.url).toLowerCase().match(/^[a-z0-9 =_.\"\'\/\?-]+$/) === null)) {
        // if null, there are bad characters
        // if the request has any characters other than alphanumeric, space, equals,
        // underscore, period, ampersand, single quote, double quote, forward slash,
        // or question mark, then we will reject it
        logger.warn(`REST AUTH - REQUEST NOT CLEAN: ${req.url}`);
        replyWithUnauthorized(req, res, '(not authenticated yet)', '(not authenticated yet)', req.url, theMessage);
    } else {
        return true;
    }
}

router.get('/*', async (req, res, callback) => {
    // console.log(`.............GET in  rest-routes 01: ${req.url}`);
    // console.log(req.rateLimit);
    // `.replace(/([^:])\/\/+/g, '$1/')` below removes extraneous slashes from the URL
    req.url = req.url.replace(/([^:])\/\/+/g, '$1/');
    if (checkThatRequestIsClean(req, res, 'unauthorized rr01')) {
        passport.authenticate('basic', { session: false }, async (err, user, info) => {
            if (err || !user) {
                replyWithUnauthorized(req, res, '(authentication failed)', ('authentication failed'), req.url, info);
            } else {
                startTheGet(req, res, user.username, user.role, req.url);
            }
        })(req, res, callback);
    }
});

router.put('/*', async (req, res, callback) => {
    // console.log(`.............PUT in  rest-routes 01: ${req.url}`);
    // console.log(req.rateLimit);
    // `.replace(/([^:])\/\/+/g, '$1/')` below removes extraneous slashes from the URL
    req.url = req.url.replace(/([^:])\/\/+/g, '$1/');
    const theURITemp = req.url.split('/');
    const theValue = theURITemp.pop(); // gets the value off the end
    const theURI = theURITemp.join('/'); // reassembles URI minus the value
    if (checkThatRequestIsClean(req, res, 'unauthorized rr01')) {
        passport.authenticate('basic', { session: false }, async (err, user, info) => {
            if (err || !user) {
                replyWithUnauthorized(req, res, '(authentication failed)', ('authentication failed'), req.url, info);
            } else {
                startThePut(req, res, user.username, user.role, theURI, theValue);
            }
        })(req, res, callback);
    }
});

processTimeoutsInQueryIDs();

module.exports = {
    restRoutes: router,
    queryIDandRes,
};
