/* eslint-disable no-underscore-dangle */
/* eslint-disable no-console */
const mongoose = require('mongoose');
// eslint-disable-next-line import/no-extraneous-dependencies, import/no-unresolved
const fims = require('fims');

const colors = require('colors');

fims.connect('dbi');
fims.subscribeTo('/dbi');

/**
 * Listen Rate in milliseconds
 * @type {number}
 */
const listenRate = 20;
// `listenRate` is usually in a config .json file but this module does not currently need
// a config file so we're simply declaring it here.

// NOTE: If you want to use a different name for the base database, the following
// string can be changed without impacting other functionality.

/**
 * Base Database Name
 * @type {string}
 */
const baseDatabaseName = 'dbi';

// create a server instance connection, then create databases as-needed on this server instance.
const databaseConnection = mongoose.createConnection(`mongodb://localhost:27017/${baseDatabaseName}`, { useNewUrlParser: true, useUnifiedTopology: true });
databaseConnection.once('open', () => {
    console.log('Connection to MongoDB established by dbi.');
});

/**
 * Contains collection of database connections
 * @type {{collectionName: Connection|object}}
 */
const databasesObject = {};

/**
 * List of collection names
 * @type {string[]}
 */
const collectionNames = [];

/**
 * Adds collection to database object and collection names
 * @param {string} collectionName
 */
function addDatabase(collectionName) {
    collectionNames.push(collectionName);
    databasesObject[collectionName] = databaseConnection.useDb(baseDatabaseName).model(`${collectionName}`, new mongoose.Schema({}, { strict: false, timestamps: true }));
}

addDatabase(baseDatabaseName);

/**
 * Translates endpoint to include array references if necessary
 * @param {string} endpoint uri to parse
 * @returns {object} object used as params in mongoose functions
 */
function parseEndpoint(endpoint) {
    console.log('parsing endpoint'.brightBlue);
    // This function is needed to handle array references in endpoints like the zeros
    // in `/assets/ess/asset_instances/0/modesInit/0/siteState` which corresponds to
    // `assets.ess.asset_instances[0].modesInit[0].siteState` in standard dot notation.
    //
    // Finding and/or updating "endpoints without arrays" and "endpoints with arrays"
    // is subtly different in both the find criteria and the update object. We run
    // through the endpoint and check each segment to see if it is an integer. An
    // alphanumeric segment with a number in it will not trigger the for loop, only a
    // lone integer. If any segment is an integer then `hasArrayRefInEndpoint` is set
    // to true.
    let findCriteria;
    let hasArrayRefInEndpoint = false;
    const endpointSplit = endpoint.split('.');
    for (let i = 0; i < endpointSplit.length; i += 1) {
        // NOTE: leave the following line with double-equals instead of triple-equals.
        // eslint-disable-next-line eqeqeq
        if (parseInt(endpointSplit[i], 10) == endpointSplit[i]) {
            hasArrayRefInEndpoint = true;
            break;
        }
    }
    if (hasArrayRefInEndpoint) {
        findCriteria = { [`${endpoint.split('.')[0]}.${endpoint.split('.')[1]}`]: { $ne: null } };
    } else {
        findCriteria = { [endpoint.split('.')[0]]: { $ne: null } };
    }
    return { findCriteria };
}

/**
 * Prints error message to console
 * @param {string} msg the message
 */
function logError(msg) {
    console.log(msg);
}

/**
 * Create's a collection or record based on fims URI
 * @param {Connection} databaseObject collection to post to
 * @param {string} endpoint endpoint to retrieve on
 * @param {object} body object to insert
 * @param {function} callback response callback
 */
function createRecord(databaseObject, endpoint, body, callback) {
    let bodyObject = body;
    if (Object.keys(body)[0] === 'value') {
        // eslint-disable-next-line prefer-destructuring
        bodyObject = Object.values(body)[0];
    }
    const createObject = { [endpoint]: bodyObject };
    try {
        databaseObject.create(createObject)
            .then((res, err) => {
                if (res) {
                    if (res._id) { // if okay, find what was inserted
                        try {
                            databaseObject.find({ _id: res._id })
                                .then((res3, err2) => {
                                    callback(res3, err2);
                                });
                        } catch (err3) {
                            logError(`ERROR 1 in dbi/createRecord: ${err3}`);
                            callback(res, err3);
                        }
                    } else {
                        logError(`ERROR 2 in dbi/createRecord: ${res}`);
                        callback(null, res);
                    }
                } else {
                    callback(res, err);
                }
            });
    } catch (err) {
        logError(`ERROR 3 in dbi/createRecord: ${err}`);
        callback(null, err);
    }
}

/**
 * Update's record based on fims URI
 * @param {Connection} databaseObject collection to update in
 * @param {string} endpoint endpoint to retrieve on
 * @param {string} id mongodb object id
 * @param {object} body object to insert
 * @param {function} callback response callback
 */
function updateRecord(databaseObject, endpoint, id, body, callback) {
    let { findCriteria } = parseEndpoint(endpoint);
    if (id) findCriteria = { _id: id };
    let bodyObject = body;
    if (Object.keys(body)[0] === 'value') {
        // eslint-disable-next-line prefer-destructuring
        bodyObject = Object.values(body)[0];
    }
    const updateObject = { $set: { [endpoint]: bodyObject } };
    try {
        databaseObject.find(findCriteria).sort({ _id: -1 }).limit(1)
            .then((res1) => {
                let nextFindCriteria;
                if (res1 && res1.length === 0) {
                    // then there's no existing record, we're doing an upsert
                    nextFindCriteria = {};
                } else {
                    nextFindCriteria = { _id: res1[0]._id };
                }
                // I did not use `findOneAndUpdate` here because I wanted to
                // split out the steps for ease of modification as this module grows
                databaseObject.updateOne(nextFindCriteria, updateObject, { upsert: true })
                    .then((res2, err2) => {
                        if (res2) {
                            if (res2.n === 1 && res2.ok === 1) { // if okay, find what was inserted
                                try {
                                    databaseObject.find(nextFindCriteria)
                                        .then((res3, err3) => {
                                            callback(res3, err3);
                                        });
                                } catch (err3) {
                                    logError(`ERROR 1 in dbi/updateRecord: ${err3}`);
                                    callback(res2, err3);
                                }
                            } else {
                                logError(`ERROR 2 in dbi/updateRecord: ${res2}`);
                                callback(null, res2);
                            }
                        } else {
                            callback(res2, err2);
                        }
                    });
            });
    } catch (err0) {
        logError(`ERROR 3 in dbi/updateRecord: ${err0}`);
        callback(null, err0);
    }
}

/**
 * Finds record based on fims URI
 * @param {Connection} databaseObject collection to search in
 * @param {string} endpoint endpoint to retrieve on
 * @param {string} id mongodb object id
 * @param {number} numberOfRecords count of records to retrieve
 * @param {function} callback response callback
 */
function findRecord(databaseObject, endpoint, id, numberOfRecords, callback) {
    let { findCriteria } = parseEndpoint(endpoint);
    if (!endpoint) findCriteria = {};
    if (id) findCriteria = { _id: id };
    try {
        databaseObject.find(findCriteria).sort({ _id: -1 }).limit(numberOfRecords)
            .then((res, err) => {
                callback(res, err);
            });
    } catch (err) {
        logError(`ERROR in dbi/findRecord: ${err}`);
    }
}

/**
 * Deletes record based on fims URI
 * @param {Connection} databaseObject collection to delete in
 * @param {string} endpoint endpoint to retrieve on
 * @param {string} id mongodb object id
 * @param {function} callback response callback
 */
const deleteRecord = async (databaseObject, endpoint, id, callback) => {
    let { findCriteria } = parseEndpoint(endpoint);
    if (!endpoint) findCriteria = {};
    if (id) findCriteria = { _id: id };
    try {
        databaseObject.deleteMany(findCriteria)
            .then((res, err) => {
                callback(res, err);
            });
    } catch (err) {
        logError(`ERROR in dbi/deleteRecord: ${err}`);
    }
};

/**
 * Retrieves all current collections in dbi
 * @param {function} callback response callback
 */
function showCollections(callback) {
    try {
        databaseConnection.db.listCollections().toArray()
            .then((res, err) => {
                const collectionNamesFromDB = [];
                for (let i = 0; i < res.length; i += 1) {
                    collectionNamesFromDB.push(res[i].name);
                }
                callback({ collections: collectionNamesFromDB }, err);
            });
    } catch (err) {
        logError(`ERROR in dbi/showCollections: ${err}`);
    }
}

/**
 * Retrieves all documents in selected collection
 * @param {Connection} databaseObject collection to retrieve from
 * @param {function} callback response callback
 */
function showDocuments(databaseObject, callback) {
    try {
        databaseObject.find({})
            .then((res, err) => {
                const theDocuments = [];
                for (let i = 0; i < res.length; i += 1) {
                    const theRes = JSON.stringify(res[i]);
                    if (Object.keys(JSON.parse(theRes))[1] === '__v') {
                        theDocuments.push(Object.keys(JSON.parse(theRes))[3]);
                    } else {
                        theDocuments.push(Object.keys(JSON.parse(theRes))[1]);
                    }
                }
                callback({ documents: theDocuments }, err);
            });
    } catch (err) {
        logError(`ERROR in dbi/showDocuments: ${err}`);
    }
}

/**
 * Retrieves all versions for the document specified by the endpoint
 * @param {Connection} databaseObject collection to retrieve from
 * @param {string} endpoint endpoint to retrieve on
 * @param {number} numberOfRecords count of records to retrieve
 * @param {function} callback response callback
 */
function showVersions(databaseObject, endpoint, numberOfRecords, callback) {
    /* eslint-disable no-param-reassign */
    endpoint = endpoint.replace('._show_versions', '');
    let { findCriteria } = parseEndpoint(endpoint);
    if (!endpoint) findCriteria = {};
    if (!numberOfRecords) numberOfRecords = 25;
    /* eslint-enable no-param-reassign */
    try {
        databaseObject.find(findCriteria).sort({ _id: -1 }).limit(numberOfRecords)
            .then((res, err) => {
                const theVersions = [];
                for (let i = 0; i < res.length; i += 1) {
                    const theRes = JSON.stringify(res[i]);
                    const jsonParseTheRes = JSON.parse(theRes);
                    theVersions.push({
                        id: jsonParseTheRes._id,
                        created: jsonParseTheRes.createdAt,
                        updated: jsonParseTheRes.updatedAt,
                    });
                }
                callback({ versions: theVersions }, err);
            });
    } catch (err) {
        logError(`ERROR in dbi/showVersions: ${err}`);
    }
}

/**
 * Checks if uri contains valid characters
 * @param {string} uri the uri
 * @returns {boolean} is valid or not
 */
function checkThatURIIsClean(uri) {
    // eslint-disable-next-line no-useless-escape
    if (uri.toLowerCase().match(/^[a-z0-9=_.\?]+$/) === null) {
        // if null, there are bad characters
        // if the request has any characters other than alphanumeric, equals, underscore, period,
        // or question mark, then we will reject it
        return false;
    }
    return true;
}

/**
 * Extracts id string from endpoint
 * @param {string} msgEndpoint message endpoint
 * @returns {string[]} array with [id, new endpoint]
 */
function checkEndpointForID(msgEndpoint) {
    let id = null;
    let newEndpoint = msgEndpoint;
    if (msgEndpoint.includes('._id=')) {
        id = msgEndpoint.split('.').pop();
        id = id.replace('_id=', '');
        newEndpoint = msgEndpoint.split('.');
        newEndpoint.pop();
        newEndpoint = newEndpoint.join('.');
        return [id, newEndpoint];
    }
    return [id, newEndpoint];
}

/**
 * Parses response object
 * @param {object} res response object
 */
function parseResponse(res) {
    const body = {};
    // NOTE: `res` needs to be stringified and then parsed so that we can get
    // a "normal" object like:
    /*
    theKeys: [ '_id', '__v', 'config' ]
    theValues: [ '602c19676bbdbefa5a081ccd',
      0,
      { assets: { feeders: [Object] } } ]
    */

    // instead of what we get when we don't do the stringify and parse:
    /*
    theKeys: [ '$__', 'isNew', 'errors', '$locals', '$op', '_doc', '$init' ]
    theValues: [ InternalCache {
        strictMode: false,
        selected: {},
            ... and lots more lines I've cut out ...
        '$options': { skipId: true, isNew: false, willInit: true, defaults: true } },
        false,
        undefined,
        {},
        null,
        { _id: 602c19676bbdbefa5a081ccd,
            __v: 0,
            config: { assets: [Object] } },
        true ]
    */

    const theRes = JSON.stringify(res);
    const theKeys = Object.keys(JSON.parse(theRes));
    const theValues = Object.values(JSON.parse(theRes));
    for (let i = 2; i < theKeys.length; i += 1) {
        // TODO: when an item is created or updated using `updateOne`, then
        // keys 0 and 1 are `_id` and `__v`, respectively. But when an item
        // is created using `create` then key 0 is `_id`, key 1 is the name
        // of the data (e.g., "assets"), and key 2 is `__v`. So when we start
        // differentiating "create" (set) and "updateOne" (put), we will have
        // to do this differently. At this time, we only care about the "real"
        // data and its key.
        const theKey = theKeys[i];
        const theValue = theValues[i];
        body[theKey] = theValue;
    }
    return body;
}

/**
 * Sends response back to fims
 * @param {string} replyTo uri to respond back to
 * @param {object} res response object
 * @param {Error} err error object
 * @param {string} endpoint endpoint used to send GETs
 * @param {boolean} dontParseResponse used for showDocuments
 * @param {function} testCallback callback function for testing
 */
function sendResponse(replyTo, res, err, endpoint, dontParseResponse, testCallback) {
    let body = {};
    if (res) {
        if (dontParseResponse) { // this is the case for showDocuments
            body = { ...body, ...res };
        } else if (Array.isArray(res)) {
            if (typeof res[0] === 'string') { // this is the case for showCollections
                for (let n = 0; n < res.length; n += 1) {
                    body = { ...body, ...parseResponse(res[n]) };
                }
            } else {
                const theRes = JSON.stringify(res);
                const theValues = Object.values(JSON.parse(theRes));
                // eslint-disable-next-line prefer-destructuring
                body = theValues[0];
            }
        } else {
            body = { ...body, ...parseResponse(res) };
        }
    } else {
        body = { ...body, ...err };
    }
    if (endpoint) {
        // if there is a endpoint in the response then we will return only the
        // object or value at that endpoint. This only applies to GETs as of February 2021.
        const endpointSplit = endpoint.split('.');
        for (let i = 0; i < endpointSplit.length; i += 1) {
            const newBody = body[endpointSplit[i]];
            if (typeof newBody === 'object') {
                body = { ...newBody };
            } else {
                // the final iteration of this loop may be a single value instead of
                // an object
                body = body[endpointSplit[i]];
            }
        }
    }
    let bodyStringified = JSON.stringify(body);
    const byteLength = Buffer.byteLength(bodyStringified, 'utf8');
    if (byteLength > 56000) bodyStringified = `{"error":{"code": 500,"message": "ERROR: dbi response has a byte length of ${byteLength}. The FIMS limit is 65536 Bytes which yields an effective byte length of about 56000. Body is too large to send via FIMS."}}`;
    // as of February 2021, the FIMS limit is 65536 Bytes
    if (typeof replyTo === 'string') {
        fims.send({
            method: 'set',
            uri: replyTo,
            replyto: null,
            body: bodyStringified,
        });
    }
    if (testCallback) testCallback(bodyStringified, err);
}

/**
 * Processes fims recieves
 * @param {object} msg fims message object
 * @param {function} testCallback callback function for testing
 */
function processEvent(msg, testCallback) {
    if (msg) {
        console.log('message: ', msg);
        const msgMethod = msg.method;
        const msgUri = msg.uri.substring(1); // remove first slash
        // NOTE: the following declaration of msgUri is an alternate way of
        // declaring it, if permissiveness is required. I'm not using it now
        // because it would be slightly more expensive to check every time.
        // const msgUri = msg.uri.substring(0, 1) === '/' ? msg.uri.substring(1) : msg.uri;
        const msgSplit = msgUri.split('/');
        msgSplit.shift(); // remove 'dbi' from the uri
        const msgCollectionName = msgSplit.shift();
        const msgEndpoint = msgSplit.join('.');
        // we're joining with dots instead of leaving it as slashes because mongo uses
        // dot notation to describe endpoints
        let databaseObject;
        if (collectionNames.includes(msgCollectionName)) {
            databaseObject = databasesObject[msgCollectionName];
        } else {
            addDatabase(msgCollectionName);
            databaseObject = databasesObject[msgCollectionName];
        }
        switch (msgMethod) {
            case 'set':
                if (checkThatURIIsClean(msgEndpoint)) {
                    createRecord(databaseObject, msgEndpoint, msg.body, (res, err) => {
                        sendResponse(msg.replyto, res, err, null, false, testCallback);
                    });
                } else {
                    const message = 'ERROR 1 in dbi/processEvent: URI contains invalid characters.';
                    sendResponse(msg.replyto, null, { error: message }, null, false, testCallback);
                    logError(message);
                }
                break;
            case 'put':
                if (checkThatURIIsClean(msgEndpoint)) {
                    const [id, newEndpoint] = checkEndpointForID(msgEndpoint);
                    updateRecord(databaseObject, newEndpoint, id, msg.body, (res, err) => {
                        sendResponse(msg.replyto, res, err, null, false, testCallback);
                    });
                } else {
                    const message = 'ERROR 2 in dbi/processEvent: URI contains invalid characters.';
                    sendResponse(msg.replyto, null, { error: message }, null, false, testCallback);
                    logError(message);
                }
                break;
            case 'get':
                if (msgCollectionName !== '_show_collections' && msgEndpoint !== '_show_documents' && !msgEndpoint.includes('._show_versions') && !msgEndpoint.includes('._id=')) {
                    // TODO: figure out how user indicates they want more than one
                    // doc returned - currently "1" below
                    findRecord(databaseObject, msgEndpoint, null, 1, (res, err) => {
                        sendResponse(msg.replyto, res, err, msgEndpoint, false, testCallback);
                    });
                } else if (msgCollectionName === '_show_collections') {
                    showCollections((res, err) => {
                        sendResponse(msg.replyto, res, err, null, true, testCallback);
                    });
                } else if (msgEndpoint === '_show_documents') {
                    showDocuments(databaseObject, (res, err) => {
                        sendResponse(msg.replyto, res, err, null, true, testCallback);
                    });
                } else if (msgEndpoint.includes('._show_versions')) {
                    // null in the next line can be changed to a user-supplied number in future
                    showVersions(databaseObject, msgEndpoint, null, (res, err) => {
                        sendResponse(msg.replyto, res, err, null, true, testCallback);
                    });
                } else if (msgEndpoint.includes('._id=')) {
                    const [id, newEndpoint] = checkEndpointForID(msgEndpoint);
                    findRecord(databaseObject, newEndpoint, id, 1, (res, err) => {
                        sendResponse(msg.replyto, res, err, newEndpoint, false, testCallback);
                    });
                }
                break;
            case 'del':
                // eslint-disable-next-line no-case-declarations
                const [id, newEndpoint] = checkEndpointForID(msgEndpoint);
                deleteRecord(databaseObject, newEndpoint, id, (res, err) => {
                    sendResponse(msg.replyto, res, err, null, false, testCallback);
                });
                break;
            default: {
                const message = 'ERROR in dbi/processEvent: no valid method in FIMS message.';
                sendResponse(msg.replyto, null,
                    { error: message }, null, false, testCallback);
                logError(message);
            }
        }
    }
}

// START FIMS listen setup
/**
 * listenBeat object that tracks updates
 * @typedef {object} listenBeat
 * @property {function} func function to call fims.receiveWithTimeout
 * @property {number} delay number of milliseconds between each beat
 * @property {Date} startTime date object of intilization time
 * @property {number} target number of milliseconds to adjust to
 * @property {boolean} started has dbi been initialized
 * @property {number} count number of beats
 */
const listenBeat = {};

/**
 * Initializes and maintains a listenBeat object
 * @param {function} func callback to self
 * @param {number} delay milliseconds between calls
 */
function listenRateBeat(func, delay) {
    if (!listenBeat.started) {
        listenBeat.func = func;
        listenBeat.delay = delay;
        listenBeat.startTime = Date.now();
        listenBeat.target = delay;
        listenBeat.started = true;
        listenBeat.count = 1;
        setTimeout(listenRateBeat, delay);
    } else {
        const elapsed = Date.now() - listenBeat.startTime;
        const adjust = listenBeat.target - elapsed;
        listenBeat.count += 1;
        listenBeat.func(listenBeat.count);
        listenBeat.target += listenBeat.delay;
        setTimeout(listenRateBeat, listenBeat.delay + adjust);
    }
}

listenRateBeat(() => {
    fims.receiveWithTimeout(500, processEvent);
}, listenRate);
// ---- END FIMS listen setup ----

// the following makes `processEvent` available to `dbi_test.js`
module.exports = {
    processEvent,
};
