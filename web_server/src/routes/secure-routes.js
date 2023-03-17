/* eslint-disable no-useless-escape */
/* eslint-disable no-console */
// this file checks first if a JWT exists in the validJWTs array in authConfig,
// then authenticates the JWT using passport and returns the userAgent that
// was stored in the JWT. We then check to see that the JWT's userAgent is the
// same as the req's user agent and confirm that the user's role is authorized
// for the requested access uri.

// use this within any .get or .post block to see how many requests we've made
// and how many are left:
// console.log(req.rateLimit);
//    or
// logger.info(req.rateLimit);

const { exec } = require('child_process');
const express = require('express');
const passport = require('passport');
/* eslint-disable import/no-extraneous-dependencies, import/no-unresolved */
const fimsApi = require('fims');
/* eslint-enable import/no-extraneous-dependencies, import/no-unresolved */
const {
    createUser,
    removeUser,
    siteAdmin,
    editUser,
    passwordAge,
    readUsers,
    readAllSiteAdmin,
    mfaCompare,
    radiusTest
} = require('../database/database');
const SiteAdmin = require("../models/siteAdmin");
const AuditLogger = require("../logging/auditLogger");
const {
    extractCookieFromReq, checkIfValidJWT, invalidateJWT,
    compareUserAgentInJWT, confirmRoleAccess, shortJWTIdentifierStartCharacter,
    shortJWTIdentifierEndCharacter,
    findAndDisconnectSocketsByJWT,
    jwtOptions,
} = require('../auth/authConfig');
const logger = require('../logging/logger');
const webServerExports = require('../web_server');
const { queryIDandRes } = require('./rest-routes');
const jwt = require('jsonwebtoken');
const path = require('path');
require('dotenv').config({
    // TODO: if the following does not work, use `path.resolve('../.env')`
    path: path.join(__dirname, '../.env'),
});
fimsApi.connect('web_server');
const router = express.Router();

const { customLog } = require('../logging/consoleTools');
let auditLogger = new AuditLogger(fimsApi);
// tracking gets and posts per second, can be viewed in "Throughput Display" in web_ui
let getRateSite = 0;
let getRateFeatures = 0;
let getRateAssets = 0;
let getRateComponents = 0;
let getRateEvents = 0;
let getRateInspector = 0;
let postRateSite = 0;
let postRateFeatures = 0;
let postRateAssets = 0;
let postRateComponents = 0;
let postRateEvents = 0;
let postRateInspector = 0;

/**
 * Attempts to execute linux command
 * @param {string} linuxCommand linux command
 * @param {function} callback function to process output
 */
function runExec(linuxCommand, callback) {
    exec(linuxCommand, (err, stdout, stderr) => {
        if (err || stderr) {
            console.error(err, stderr);
        } else {
            callback(stdout);
        }
    });
}

/**
 * Logs authentication checks
 * @param {string} userName username
 * @param {string} role role
 * @param {string} accessPoint parent endpoint
 * @param {*} host host
 * @param {*} userAgent user agent
 * @param {*} message message
 */
function logAuthentication(userName, role, accessPoint, host, userAgent, message) {
    const theMessage = `SECURE AUTH - username: ${userName}, role: ${role}, accessPoint: ${accessPoint}, host: ${host}, user-agent: ${userAgent} ${message ? `\n${message}` : ''}`;
    logger.info(theMessage);
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
    logger.warn(`SECURE AUTH - unauthorized ERROR: ${info}`);
    logger.warn(`SECURE AUTH - unauthorized username: ${userName}, role: ${role}, accessPoint: ${accessPoint}`);
    logger.warn(`SECURE AUTH - unauthorized host: ${req.headers.host}, user-agent: ${req.headers['user-agent']}`);
    logger.info(`SECURE AUTH - unauthorized ERROR: ${info}. See warnLog for more information.`);
    invalidateJWT(extractCookieFromReq(req), res);
    return false;
}

/**
 * Uses fims to send a GET request
 * @param {object} req request object
 * @param {object} res response object
 * @param {object} err error object
 * @param {string} userName username
 * @param {string} role role
 * @param {*} userAgent user agent
 * @param {string} uri path for GET
 * @param {string} replyTo fims uri to send reply
 * @param {object} body body for fims send
 */
function wrapUpGet(req, res, err, userName, role, userAgent, uri, replyTo, body) {
    const roleAccessPayload = {
        userName, 
        role, 
        accessPoint:uri,
        req,
        res
    };
    
    if (compareUserAgentInJWT(req, res, userName, userAgent) && 
    (confirmRoleAccess("readWrite", roleAccessPayload) || confirmRoleAccess("read", roleAccessPayload))) {
        if (userName) {
            // this section sends the request to FIMS
            const theURI = uri.replace('/api', '');
            const msg = {
                method: 'get',
                uri: theURI,
                replyto: replyTo || `/ui${theURI}`,
                body,
                username: userName
            };
            fimsApi.send(msg);
            switch (theURI.split('/')[1]) {
                case 'site':
                    getRateSite += 1;
                    break;
                case 'features':
                    getRateFeatures += 1;
                    break;
                case 'assets':
                    getRateAssets += 1;
                    break;
                case 'events':
                    getRateEvents += 1;
                    break;
                case 'components':
                    getRateComponents += 1;
                    break;
                case 'inspector':
                    getRateInspector += 1;
                    break;
                case 'inspectorControl':
                    getRateInspector += 1;
                    break;
                default:
                    break;
            }

            // this section sends the acknowledgement to the user
            const status = 200;
            const message = `API: JWT verified, accessing ${uri}`;
            logAuthentication(userName, role, uri, req.headers.host, req.headers['user-agent'], message);
            if (!theURI.includes('/dbi/') && !theURI.includes('/fleet')) {
                res.json({
                    status,
                    message,
                    user: userName,
                });
            }
        }
    } else {
        replyWithUnauthorized(req, res, userName, role, uri, 'unauthorized sr01');
    }
}


/**
 * GET request to local mongo database, not proxy for fims
 * @param {object} req request object
 * @param {object} res response object
 * @param {object} err error object
 * @param {string} userName username
 * @param {string} role role
 * @param {*} userAgent user agent
 * @param {string} uri path for GET
 * @param {string} replyTo fims uri to send reply
 * @param {object} body body for fims send
 */
 function wrapUpMongoGet(req, res, err, userName, role, userAgent, uri, replyTo, body) {
    const roleAccessPayload = {
        userName, 
        role, 
        accessPoint:uri,
        req,
        res
    };
    
    if (compareUserAgentInJWT(req, res, userName, userAgent) && 
    (confirmRoleAccess("readWrite", roleAccessPayload) || confirmRoleAccess("read", roleAccessPayload))) {
        if (userName) {
           
            const status = 200;
            const message = `API: JWT verified, accessing ${uri}`;
            logAuthentication(userName, role, uri, req.headers.host, req.headers['user-agent'], message);
            
            res.json({
                status,
                message: body,
                user: userName,
            });
            
        }
    } else {
        replyWithUnauthorized(req, res, userName, role, uri, 'unauthorized sr01');
    }
}

/**
 * Check's JWT and authentication, then calls GET function
 * @param {object} req request object
 * @param {object} res response object
 * @param {string} theURI URI path for GET
 * @param {string} theReplyTo URI path to reply to
 * @param {object} theBody body object
 */
function startTheGet(req, res, theURI, theReplyTo, theBody) {
    if (checkIfValidJWT(extractCookieFromReq(req), 'api request', req, res)) {
        // eslint-disable-next-line consistent-return
        passport.authenticate('jwt-cookiecombo', (err, payload) => {
            if (theURI.includes('/dbi/')) {
                const queryID = Date.now() + Math.floor(100000 + Math.random() * 900000);
                queryIDandRes.push({ queryID, res, timeout: Date.now() + 60000 });
                const msg = {
                    method: 'get',
                    uri: req.url,
                    replyto: `/rest${req.url}/${queryID}`,
                    body: {},
                    username: payload.user
                };
                fimsApi.send(msg);
            }
            if (theURI.includes('/site-admin/read/summary')){
                
                readAllSiteAdmin(req, (error, result)=>{
                    
                    let userMessage = !error ? result : error;
                    wrapUpMongoGet(req, res, err, payload.user, payload.role, payload.userAgent,
                        theURI, theReplyTo, userMessage);

                    return;
                });
            } else if (theURI.includes('/users/read/summary')){
                readUsers(req.query, (error, result)=>{

                    let userMessage = !error ? result : error;
                    wrapUpMongoGet(req, res, err, payload.user, payload.role, payload.userAgent,
                        theURI, theReplyTo, userMessage);
                    return;
                });

            } else if (theURI.includes('/fleet')){
                const queryID = Date.now() + Math.floor(100000 + Math.random() * 900000);
                queryIDandRes.push({ queryID, res, timeout: Date.now() + 60000 });
                const replyToURI = '/rest/fleet/features/'+ queryID
                wrapUpGet(req, res, err, payload.user, payload.role, payload.userAgent,
                    theURI, replyToURI, theBody);
            }
             else {
                wrapUpGet(req, res, err, payload.user, payload.role, payload.userAgent,
                    theURI, theReplyTo, theBody);
            }
        })(req, res);
    }
}

/**
 * SETs or PUTs value through fims
 * @param {object} res response object
 * @param {string} uri path for GET
 * @param {*} value value to POST
 * @param {boolean} isPut if true then command is PUT instead of SET
 * @param {string} userName 
 */
function wrapUpPost(res, uri, value, isPut, userName) {
    // for updating property values
    let body;
    if (typeof value === 'object' && !uri.includes("fleet")) {
        body = value;
    } 
    else if (uri.includes("fleet")){
        body = value["value"]
    }
    else {
        body = `{"value":${value}}`;
    }

    const method = isPut ? 'put' : 'set';
    const msg = {
        method,
        uri,
        replyto: null,
        body,
        username: userName
    };
    fimsApi.send(msg);
    switch (uri.split('/')[1]) {
        case 'site':
            postRateSite += 1;
            break;
        case 'features':
            postRateFeatures += 1;
            break;
        case 'assets':
            postRateAssets += 1;
            break;
        case 'events':
            postRateEvents += 1;
            break;
        case 'components':
            postRateComponents += 1;
            break;
        case 'inspector':
            postRateInspector += 1;
            break;
        case 'inspectorControl':
            postRateInspector += 1;
            break;
        default:
            break;
    }
    if (typeof value === 'object') {
        res.status(200).send(value);
    } else {
        res.status(200).send('{}');
    }
}

/**
 * Route for posting specifically for admins
 * @param {object} response response message object
 * @param {object} req request object
 * @param {object} res response object
 * @param {string} userName username
 * @param {*} role role
 * @param {*} action action
 */
function wrapUpPostUserAdmin(response, req, res, userName, role, action, actor) {
    // for user administration - creating and removing users
    let message = 'API: JWT verified, ';
    const errorMessages = [];
    let status = 200;

    if (response.message) {
        if (response.errors) {
            Object.keys(response.errors).forEach((error) => {
                errorMessages.push(response.errors[error].message);
            });
        } else {
            errorMessages.push(response.message);
        }
        message += `${actor} NOT ${action.toUpperCase()}ED: ${errorMessages.join(', ')}`;
        status = 400;
    } else {
        message += `${actor} "${response.username}" ${action}ed`;
        const trackerData = {username: userName, userrole: role, modified_field: actor, modified_value: `${action}ed`}
        auditLogger.send(trackerData);
    }
    logAuthentication(userName, role, '/administration/useradministration', req.headers.host, req.headers['user-agent'], message);
    res.json({
        status,
        message,
        user: userName,
    });
}

/**
 * Checks for validation on POST request and passes params
 * @param {object} req request object
 * @param {object} res response object
 * @param {object} theRequest also request object?
 * @param {string} theURI path for POST
 * @param {*} theValue value to POST
 * @param {boolean} isPut if true then command is PUT instead of SET
 */
function startThePost(req, res, theRequest, theURI, theValue, isPut) {
    if (checkIfValidJWT(extractCookieFromReq(req), 'api request', req, res)) {
        passport.authenticate('jwt-cookiecombo', async (err, payload) => {
            const endpoint = (theURI) ? theURI : theRequest;
            const roleAccessPayload = {
                userName: payload.user, 
                role:payload.role, 
                accessPoint: endpoint,
                req:req,
                res:res
            };
            if (compareUserAgentInJWT(req, res, payload.user, payload.userAgent) && confirmRoleAccess("readWrite", roleAccessPayload)) {
                switch (theRequest) {
                    case 'updatePropertyValue':
                        wrapUpPost(res, theURI, theValue, isPut, payload.user);
                        break;
                    case 'createuser':
                        createUser(req.query, (error, response) => {
                            wrapUpPostUserAdmin(error || response, req, res, payload.user, payload.role, 'creat', 'user');
                        });
                        break;
                    case 'removeuser':
                        removeUser(req.query, (error, response) => {
                            wrapUpPostUserAdmin(error || response, req, res, payload.user, payload.role, 'remov', 'user');
                        });
                        break;
                    case 'edituser':
                        editUser(req.query, (error, response) => {
                            wrapUpPostUserAdmin(error || response, req, res, payload.user, payload.role, 'edit', 'user');
                        });
                        break;
                    case 'site-admin':
                        siteAdmin(req.body, (error, response) => {
                            wrapUpPostUserAdmin(error || response, req, res, payload.user, payload.role, 'complet', 'changes to siteadmin');
                        });
                        break;
                    case 'mfa':
                        mfaCompare(req.query, (error, response) => {
                            wrapUpPostUserAdmin(error || response, req, res, payload.user, payload.role, 'complet', 'changes to MFA');
                        });
                        break;
                    case 'radius_test':
                        radiusTest(req.body, (error, response) => {
                            wrapUpPostUserAdmin(error || response, req, res, payload.user, payload.role, 'radius_test', 'user');
                        });
                        break;
                    default:
                        replyWithUnauthorized(req, res, payload.user, payload.role, theURI, 'unauthorized sr02');
                }
            } else {
                replyWithUnauthorized(req, res, payload.user, payload.role, theURI, 'unauthorized sr02'); 
            } 

        })(req, res);
    }
}

/**
 * Logs logout command
 * @param {object} req request object
 * @param {object} res response object
 */
function doLogout(req, res) {
    const theJWT = extractCookieFromReq(req);
    logger.info(`SECURE AUTH - LOGOUT: received logout request from JWT ${theJWT.substring(shortJWTIdentifierStartCharacter, shortJWTIdentifierEndCharacter)}`);
    res.json({
        status: 200,
        message: 'AUTH: user has logged out',
        username: req.user,
    });
    const decoded = jwt.verify(theJWT, process.env.JWT_SECRET)
    if (decoded) invalidateJWT(theJWT);
    auditLogger.send({modified_field:"LOG OUT", modified_value: true, username: decoded.user, userrole: decoded.role});
    logger.info(`SECURE AUTH - LOGOUT: logout request from JWT ${theJWT.substring(shortJWTIdentifierStartCharacter, shortJWTIdentifierEndCharacter)} COMPLETED`);
}

// eslint-disable-next-line consistent-return
/**
 * Checks if request is valid
 * @param {object} req request object
 * @param {object} res response object
 * @param {*} theRequest request command
 * @param {*} theMessage message for request
 * @returns {boolean} true if valid request
 */
function checkThatRequestIsClean(req, res, theRequest, theMessage) {
    if ((theRequest.length > 1000) || (decodeURIComponent(theRequest).toLowerCase().match(/^[a-z0-9 =_.\-\"\'\/\?]+$/) === null)) {
        logger.warn(`SECURE AUTH - REQUEST NOT CLEAN: ${theRequest}`);
        replyWithUnauthorized(req, res, 'null', 'null', theRequest, theMessage);
    } else {
        return true;
    }
}

// eslint-disable-next-line consistent-return
/**
 * Checks if request is valid for specific database
 * @param {object} req request object
 * @param {object} res response object
 * @param {*} theRequest request command
 * @param {*} theMessage message for request
 * @param {string} databaseType determines if mongo or influx
 * @returns {boolean} true if valid request for database
 */

function checkThatDatabaseRequestIsClean(req, res, theRequest, theMessage, databaseType) {

    const theRequestLowerCase = theRequest.toLowerCase();

    if ((theRequestLowerCase.match(/drop|delete|truncate/) === null) && (theRequestLowerCase.length < 1001)) {

        if (databaseType === 'influx') {

            if (decodeURIComponent(theRequestLowerCase).match(/^[a-z0-9 =_.;\"\'\/\\\?]+$/) === null) {

                logger.warn(`SECURE AUTH - INFLUX REQUEST NOT CLEAN 01: ${theRequest}`);
                replyWithUnauthorized(req, res, 'null', 'null', theRequest, theMessage);

            } else {

                return true;

            }

        } else if (decodeURIComponent(theRequestLowerCase).match(/^[a-z0-9 =_.,\-:\[\]\{\}\"\'\/\?]+$/) === null) {

            logger.warn(`SECURE AUTH - INFLUX REQUEST NOT CLEAN 02: ${theRequest}`);
            replyWithUnauthorized(req, res, 'null', 'null', theRequest, theMessage);

        } else {

            return true;

        }
    } else {

        logger.warn(`SECURE AUTH - INFLUX REQUEST NOT CLEAN 03: ${theRequest}`);
        replyWithUnauthorized(req, res, 'null', 'null', theRequest, theMessage);

    }
}

// authenticate token, if already exists when user opens login page
router.get('/authenticate-user-token', (req, res) => {
    try {
        const token = extractCookieFromReq(req);
        if (token !== "no_JWT_found"){

            if (checkIfValidJWT(token, 'api request', req, res)) {
                
                jwt.verify(token, process.env.JWT_SECRET, (err, decoded)=>{

                    if (!err){
                        SiteAdmin.findOne({}, (err, site)=>{
                        
                        if (site && !err){
                            if (site.password.password_expiration){
                                passwordAge(site.password.password_expiration_interval, decoded.user, (status)=>{

                                    if (!status.hasExpired){
                                        res.json({role: decoded.role, user: decoded.user, password_expired_status: status})
                                    }
                                })
                            }
                        }
                        
                    })
                    auditLogger.send({modified_field: "authentication token login", modified_value: true, username: decoded.user, userrole: decoded.role});
                    res.json({role: decoded.role, user: decoded.user})

                    } else {
                        logger.warn(`JWT TOKEN DID NOT VERIFY: ${err}`);
                    }

                });

            } else {
                res.json({role: null })
            }

        }
    } catch (error) {
        logger.warn("there was an error parsing the existing token")
    }
});

router.get('/dbi/:database/:collection', (req, res) => {
    if (checkThatDatabaseRequestIsClean(req, res, req.url, 'unauthorized sr12', 'dbi')) {
        startTheGet(req, res, req.url, null, {});
    }
});

router.put('/dbi/:database/:collection', (req, res) => {
    if (checkThatDatabaseRequestIsClean(req, res, req.url, 'unauthorized sr13', 'dbi')) {
        startThePost(req, res, 'updatePropertyValue', req.url, req.body, true);
    }
});

router.post('/dbi/:database/:collection', (req, res) => {
    if (checkThatDatabaseRequestIsClean(req, res, req.url, 'unauthorized sr14', 'dbi')) {
        // 'true' at the end of the following line makes the method 'put'
        startThePost(req, res, 'updatePropertyValue', req.url, req.body);
    }
});

router.get('/fleet/:features/:fleet_name/overridable', (req, res) => {
    const theURI = `/fleet/${req.params.features}/${req.params.fleet_name}/overridable`;
    // console.log(`.............GET in secure-routes 07: ${theURI}`);
    if (checkThatRequestIsClean(req, res, req.url, 'unauthorized sr05')) {
        startTheGet(req, res, theURI, null, null);
    }
});

router.get('/fleet/:features/:fleet_name/:site_name/overridable', (req, res) => {
    const theURI = `/fleet/${req.params.features}/${req.params.fleet_name}/${req.params.site_name}/overridable`;
    // console.log(`.............GET in secure-routes 07: ${theURI}`);
    if (checkThatRequestIsClean(req, res, req.url, 'unauthorized sr05')) {
        startTheGet(req, res, theURI, null, null);
    }
});

router.get('/fleet/:features/:fleet/:site/:variable', (req, res) => {
    const theURI = `/fleet/${req.params.features}/${req.params.fleet}/${req.params.site}/${req.params.variable}`;
    // console.log(`.............GET in secure-routes 07.5: ${theURI}`);
    if (checkThatRequestIsClean(req, res, req.url, 'unauthorized sr05')) {
        startTheGet(req, res, theURI, null, null);
    }
});

router.get('/:base_uri/:type/:id/:value', (req, res) => {
    const theURI = `/${req.params.base_uri}/${req.params.type}/${req.params.id}/${req.params.value}`;
    // console.log(`.............GET in secure-routes 04: ${theURI}`);
    if (checkThatRequestIsClean(req, res, req.url, 'unauthorized sr05')) {
        startTheGet(req, res, theURI, null, null);
    }
});

router.get('/:base_uri/:type/:summary', (req, res) => {
    const theURI = `/${req.params.base_uri}/${req.params.type}/summary`;
    // console.log(`.............GET in secure-routes 01: ${theURI}`);
    if (checkThatRequestIsClean(req, res, req.url, 'unauthorized sr05')) {
        startTheGet(req, res, theURI, null, null);
    }
});

router.get('/:base_uri/:type', (req, res) => {
    let theURI = `/${req.params.base_uri}`;
    let theBody = null;
    // console.log(`.............GET in secure-routes 02: ${theURI}`);
    if (req.params.base_uri === 'events') {
        if (checkThatDatabaseRequestIsClean(req, res, req.url, 'unauthorized sr06', 'mongo')) {
            theBody = decodeURIComponent(req.params.type);
            startTheGet(req, res, theURI, null, theBody);
        }
    } else if (checkThatRequestIsClean(req, res, req.url, 'unauthorized sr06.5')) {
        if (req.params.type) theURI += `/${req.params.type}`;
        startTheGet(req, res, theURI, null, theBody);
    }
});


router.get('/:base_uri', (req, res) => {
    const theURI = `/${req.params.base_uri}`;
    // console.log(`.............GET in secure-routes 03: ${theURI}`);
    if (checkThatRequestIsClean(req, res, req.url, 'unauthorized sr07')) {
        startTheGet(req, res, theURI, null, null);
    }
});

router.post('/radius_test', (req, res) => {
    startThePost(req, res, 'radius_test', '/radius_test', req.body, false);
})

router.post('/fleet/:features/:fleet/:site/:variable', (req, res) => {
    const theURI = `/fleet/${req.params.features}/${req.params.fleet}/${req.params.site}/${req.params.variable}`;
    //console.log(`.............POST in secure-routes 07: ${theURI}`);
    if (checkThatRequestIsClean(req, res, req.url, 'unauthorized sr08')) {
        startThePost(req, res, 'updatePropertyValue', theURI, req.body);
    }
});

router.post('/:base_uri/:type/:id/:property/:value', (req, res) => {
        const theURI = `/${req.params.base_uri}/${req.params.type}/${req.params.id}/${req.params.property}`;
        // console.log(`.............POST in secure-routes 01: ${theURI}`);
        if (checkThatRequestIsClean(req, res, req.url, 'unauthorized sr08')) {
            startThePost(req, res, 'updatePropertyValue', theURI, req.params.value);
        }
});

router.post('/inspector/:module/:replyName/:value', (req, res) => {
    // console.log(' ************* in post /inspector');
    if (checkThatRequestIsClean(req, res, req.url, 'unauthorized sr09')) {
        const theURI = `/inspector/${req.params.module}/${req.params.replyName}`;
        startThePost(req, res, 'updatePropertyValue', theURI, req.params.value);
    }
});

router.post('/:base_uri/:type/:id/:value', (req, res) => {
    const theURI = `/${req.params.base_uri}/${req.params.type}/${req.params.id}`;
    // console.log(`.............POST in secure-routes 02: ${theURI}`);
    if (checkThatRequestIsClean(req, res, req.url, 'unauthorized sr10')) {
        startThePost(req, res, 'updatePropertyValue', theURI, req.params.value);
    }
});

router.post('/:base_uri/:type/:id', (req, res) => {
    const theURI = `/${req.params.base_uri}/${req.params.type}/${req.params.id}`;
    // console.log(`.............POST in secure-routes 05: ${theURI}`);
    if (checkThatRequestIsClean(req, res, req.url, 'unauthorized sr10')) {
        startThePost(req, res, 'updatePropertyValue', theURI, req.body, false);
    }
});

// Don't know if this actually used
router.post('/:base_uri/:type', (req, res) => {
    const theURI = `/${req.params.base_uri}/${req.params.type}`;
    // console.log(`.............POST in secure-routes 04: ${theURI}`);
    if (checkThatRequestIsClean(req, res, req.url, 'unauthorized sr10')) {
        if (req.params.base_uri === 'site-admin') {
            startThePost(req, res, req.params.base_uri, theURI, req.body, false);
        } else {
            startThePost(req, res, 'updatePropertyValue', theURI, req.body, false);
        }
    }
});

router.post('/:request', (req, res) => {
    const theRequest = req.params.request;
    // console.log(`.............POST in secure-routes 03: ${theRequest}`);
    if (checkThatRequestIsClean(req, res, theRequest, 'unauthorized sr11')) {
        if (theRequest === 'logout') {
            doLogout(req, res);
        } else {
            startThePost(req, res, theRequest, null, null);
        }
    }
});

module.exports = {
    secureRoutes: router,
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
    checkThatRequestIsClean,
    checkThatDatabaseRequestIsClean,
};