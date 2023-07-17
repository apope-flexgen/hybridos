const utils = require('./utils')
/* eslint-disable no-console */
/* eslint-disable no-use-before-define */
const fs = require('fs');
const path = require('path');
const crypto = require('crypto');

const hash = crypto.createHash('sha256');
hash.setEncoding('hex');
// eslint-disable-next-line import/no-unresolved
const fims = require('@flexgen/fims');
const comp = require('./computations');

/**
 * Path to metrics configuration JSON
 * @type {string}
 */
const pathArg = process.argv[2];
if (!pathArg) {
    throw new Error('Please supply a path to .json configuration file. Usage: node metrics.js path/to/config/');
}
/**
 * Absolute path to config directory
 * @type {string}
 */
let configPath = path.resolve(pathArg);

/**
 * Path to config file
 * @type {string}
 */
let configPathAndFile = configPath.includes('.json') ? configPath : path.join(configPath, 'metrics.json');
// If configPathAndFile exists then we are good-to-go. Otherwise:
if (!fs.existsSync(configPathAndFile)) {
    // If we get here then no `metrics.json` exists at that path. Maybe it is a path with a
    // filename as follows:
    // path argument received: /Users/dmullen/hybridos/config/fleet_manager/metrics/siteName
    // resulting path to file: /Users/dmullen/hybridos/config/fleet_manager/metrics/siteName.json
    // So we will try that:
    configPathAndFile = configPath.concat('.json');
    if (!fs.existsSync(configPathAndFile)) {
        // If we get here then no `[sitename].json` exists at that location. Maybe the argument
        // provided is expecting `_metrics` to be appended to the filename as follows:
        // path argument received: path/to/config/fleet_manager/metrics/siteNameMetrics
        // resulting path to file: path/to/config/fleet_manager/metrics/siteNameMetrics_metrics.json
        // So we will try that:
        configPathAndFile = configPath.concat('_metrics.json');
        if (!fs.existsSync(configPathAndFile)) {
            // if we get here then we will take another approach and see if the last segment
            // of the provided URL is part of the name of the .json file itself such as
            // path/to/config/metrics/odessa expecting to find odessa.json or odessa_metrics.json
            configPathAndFile = `${configPath}/${configPath.replace(/\/+$/, '').split('/').pop()}${'.json'}`;
            if (!fs.existsSync(configPathAndFile)) {
                configPathAndFile = `${configPath}/${configPath.replace(/\/+$/, '').split('/').pop()}${'_metrics.json'}`;
                if (!fs.existsSync(configPathAndFile)) {
                    // If we get here then none of the above combinations worked
                    throw new Error(`.json configuration file not found in ${configPath}. Please supply a path to .json configuration file. Usage: node metrics.js path/to/config/`);
                }
            }
        }
    }
}

// get the path where the metrics json is so we can put the mdo json in that same directory.
// This handles situations where the configPath arg was supplied with a file name at the end
// (e.g. "metrics.json")

/**
 * Temporary array of path components
 * @type {string[]}
 */
const configPathTemp = configPathAndFile.split('/');

/**
 * Name of metrics config file including filetype (should be .json)
 * @type {string}
 */
const configFileName = configPathTemp.pop();
configPath = configPathTemp.join('/');

/**
 * Argument used for testing
 * @type {string}
 */
const processArgv3 = process.argv[3];
// If we have 'pathTest' as the third arg when starting metrics
// then we report the path argument received and the resulting path, then exit.
// If the third arg is 'test' then we do some actions at the bottom of this file.
if (processArgv3 === 'pathTest') {
    console.log(`\npath argument received: ${process.argv[2]}`);
    console.log(`path derived from that: ${configPath}`);
    console.log(`      config file name: ${configFileName}`);
    console.log('      path to mdo file:', configPath.concat(`/mdo_${configFileName}`));
    console.log(`   path to config file: ${configPathAndFile}\n\n`);
    process.exit();
}

// If we have 'computationTest' as the third arg when starting metrics
// then we do the computation, then exit.
if (processArgv3 === 'computationTest') {
    // use or modify any of these for a quick test of any computation
    // const theParams = { operation: 'lt', reference: 60000 }
    // console.log('compareMillisecondsToCurrentTime:',
    //    comp.compareMillisecondsToCurrentTime([1605724239388], null, theParams))

    const theParams = { operation: 'timezone', reference: -4 };
    console.log('millisecondsToRFC3339', comp.millisecondsToRFC3339(Date.now(), null, theParams));

    // const theParams = { operation: 'gt' };
    // console.log('compare:', comp.compare([11, 2], null, theParams));
    // const theCurrentTime = 1605197080690;
    // const theSavedTime = 1605197080685;
    // console.log('srff:', comp.srff([((theSavedTime + 5) > theCurrentTime), null]));

    // const theParams = { offset: 3 };
    // console.log('sum:', comp.sum([1], null, theParams));

    // console.log('date:', comp.millisecondsToRFC3339(1605197080690));
    process.exit();
}

// ---- INITIALIZATION ----
/**
 * Master data object for metrics
 * @type {object}
 */
const mdo = {};
/**
 * Pub and set mapping object for URIs
 * @type {object}
 */
const psmap = {};
/**
 * Holds list of URIs for fims to subscribe to
 * @type {string[]}
 */
let subscribes = [];
/**
 * Path location of mdo json file
 * @type {string}
 */
const mdoJsonPathAndFile = configPath.concat(`/mdo_${configFileName}`);
// the mdo will be written to the mdo json every 10 seconds. We will wait a few
// seconds for this first write, allowing things to start up.
/**
 *  Time in milliseconds of next mdo write
 * @type {number}
 */
let nextMdoWrite = Date.now() + 5000;
// eslint-disable-next-line import/no-dynamic-require
const { publishRate, listenRate, publishUris } = require(configPathAndFile);
// publishUris is used to configure the mdo below. We hash publishUris when we
// read it from metrics.json so that we can compare the hash to the hash recorded
// in the mdo json. This way we know if the publishUris that created the mdo in
// the mdo json is the same as the current publishUris
hash.write(JSON.stringify(publishUris));
hash.end();
const publishUrisHash = hash.read();

fims.connect('metrics');

// If 1) there is a saved `mdo_[filename].json` and 2) a check of the publishUrisHash
// tells us it was created based on the same metrics json file in use now, then we
// will Object.assign the mdo and psmap to what's in the mdo json file. Otherwise,
// we will configure the mdo from the config file.
if (fs.existsSync(mdoJsonPathAndFile)) {
    console.log('\n+++++ saved metrics mdo file recognized...');
    fs.stat(mdoJsonPathAndFile, (err, stats) => {
        if (err) consoleLogAndConfigureMdo(`  +++ ERROR: saved metrics mdo file may be corrupted: ${err}\n+++++ Configuring metrics mdo from config file data...\n`);
        const minutesSinceFileModified = Math.round((((new Date() - new Date(stats.mtime))
            / 1000) / 60) * 100) / 100;
        if (minutesSinceFileModified < 5) {
            console.log(`  +++ file is FRESH. File last modification time: ${stats.mtime}, ${minutesSinceFileModified} minutes old`);
            let theMdoFromDisk;
            try {
                // eslint-disable-next-line global-require, import/no-dynamic-require
                theMdoFromDisk = require(mdoJsonPathAndFile);
                // if the publishUris hash in this mdo json file does not match the current
                // publishUris then we won't use the mdo json
                if (theMdoFromDisk.publishUrisHash === publishUrisHash) {
                    try {
                        Object.assign(mdo, theMdoFromDisk.mdo);
                        Object.assign(psmap, theMdoFromDisk.psmap);
                        processSubscribes();
                        console.log('  +++ mdo file data read successfully. Configuring mdo from mdo file data...\n');
                        // use the following in testing to see the mdo right after reading from
                        // an mdo json
                        // fs.writeFileSync(configPath.concat('/mdo_save_file_read.json'),
                        //     JSON.stringify(mdo));
                    } catch (err2) {
                        // if the file doesn't contain a valid JSON object, it will be caught here
                        // `err2` will display info about the JSON error.
                        consoleLogAndConfigureMdo(`  +++ ERROR: saved metrics mdo file data was not read successfully: ${err2}\n+++++ Configuring metrics mdo from config file data...\n`);
                    }
                } else {
                    consoleLogAndConfigureMdo('  +++ NOTE: the saved metrics mdo file is for a different version of metrics.json so it will not be used.\n+++++ Configuring metrics mdo from config file data...\n');
                }
            } catch (err3) {
                // if there is an error reading the file, it will be caught here
                // `err3` will display info about the error in reading the file.
                consoleLogAndConfigureMdo(`  +++ ERROR: saved metrics mdo file data was not read successfully: ${err3}\n+++++ Configuring metrics mdo from config file data...\n`);
            }
        } else {
            consoleLogAndConfigureMdo(`  +++ file is STALE. file last modification time: ${stats.mtime}, ${minutesSinceFileModified} minutes old\n+++++ Configuring metrics mdo from config file data...\n`);
        }
    });
} else {
    consoleLogAndConfigureMdo('\n+++++ Configuring metrics mdo from config file data...\n');
}

/**
 * Logs a message then configures the master data object
 * @param {string} msg message to be printed
 */
function consoleLogAndConfigureMdo(msg) {
    console.log(msg);
    configureMdo();
}

/**
 * Configures the master data object
 * 1. Parse through top level URIs that metrics is publishing as
 * 2. For each entry, copy the publish parameters that exist
 * 3. Initialize input and metric values according to the metric and config
 * 4. Insert the entry into the master data object (mdo) with insertEntry()
 */
function configureMdo() {
    // Configure master data object
    // 1. Parse through top level URIs that metrics is publishing as
    // 2. For each entry, copy the publish parameters that exist
    // 3. Initialize input and metric values according to the metric and config
    // 4. Insert the entry into the master data object (mdo) with insertEntry()
    // which does some cleanup
    publishUris.forEach((u) => {
        const pubUri = u.uri;
        mdo[pubUri] = {};
        u.metrics.forEach((m) => {
            if (mdo[pubUri][m.id]) throw new Error(`Duplicate metric detected - ${m.id}`);
            // console.log(`Configuring metric - ${m.id}`);
            const op = m.operation;
            let value; let
                inputValue;

            // Some things are pass through
            const entry = {
                name: m.name,
                scale: m.scale,
                unit: m.unit,
                ui_type: m.ui_type,
                type: m.type,
                options: m.options,
                operation: op,
                naked: u.naked,
            };
            if (m.outputs) entry.outputs = m.outputs;
            if (m.param) entry.param = m.param;

            // Initialize value and inputs' values
            // Integrate has state, and we'd like to pre-populate if possible
            // There may be a better way to segment this switch statement to avoid
            // the redundant looking calls to insertEntry
            /* eslint-disable no-prototype-builtins */
            switch (op) {
                // TODO: Figure out if some of this code can be pushed into computations.js
                // as callbacks so that you potentially don't have to touch metrics.js if
                // you need to do some custom initialization of a computation.
                case 'integrate':
                    requestValueFromStorage(m.id, pubUri, (lastValue) => {
                        // eslint-disable-next-line no-param-reassign
                        if (lastValue === 'no response received') lastValue = 0;
                        // we could use this 'no response received' to trigger other events
                        // or logging
                        console.log(`value received from Influx for ${m.id} is ${lastValue}`);
                        value = lastValue || (m.hasOwnProperty('initialValue') ? m.initialValue : 0);
                        entry.state = value ? { value } : { value: 0 };
                        inputValue = 0;
                        insertEntry(m, pubUri, value, entry, inputValue);
                    });
                    break;
                case 'bitmask': // Bitmask metrics take bit field inputs ([]) and turn them into booleans
                case 'and':
                case 'compare':
                case 'bitfieldpositioncount':
                    value = m.hasOwnProperty('initialValue') ? m.initialValue : false;
                    inputValue = m.hasOwnProperty('initialInput') ? m.initialInput : [];
                    insertEntry(m, pubUri, value, entry, inputValue);
                    break;
                case 'bitfield': // Bitfield metrics take boolean inputs and turn them into bit fields
                    value = m.hasOwnProperty('initialValue') ? m.initialValue : [];
                    inputValue = m.hasOwnProperty('initialInput') ? m.initialInput : false; // TODO hold on to name string
                    insertEntry(m, pubUri, value, entry, inputValue);
                    break;
                case 'srff':
                    value = m.hasOwnProperty('initialValue') ? m.initialValue : false;
                    entry.state = { q: value };
                    inputValue = m.hasOwnProperty('initialInput') ? m.initialInput : false;
                    insertEntry(m, pubUri, value, entry, inputValue);
                    break;
                case 'pulse':
                    value = m.hasOwnProperty('initialValue') ? m.initialValue : false;
                    entry.state = { value };
                    inputValue = m.hasOwnProperty('initialInput') ? m.initialInput : false;
                    insertEntry(m, pubUri, value, entry, inputValue);
                    break;
                default:
                    value = m.hasOwnProperty('initialValue') ? m.initialValue : 0;
                    inputValue = m.hasOwnProperty('initialInput') ? m.initialInput : 0;
                    insertEntry(m, pubUri, value, entry, inputValue);
            }
            /* eslint-enable no-prototype-builtins */
        });
    });
    // use the following in testing to see the mdo right after it is created from the config file
    // fs.writeFileSync(configPath.concat('/config_file_read.json'), JSON.stringify(mdo));
    processSubscribes();
    return mdo
}

// insertEntry() does some cleanup on the entries parsed in the initialization phase.
// 1. Initialize the metric value with the supplied `value`
// 2. Conform `inputs` URIs to the form { uri: "/frag0/.../fragn-1", id: "fragn"}
// 3. Replace input list in entry with an object where the fully qualified URI is used as
//    the key. The order of `inputs` stays the same as insertion order when used later
// 4. Initialize input values in the metric entry with the supplied `inputValue`
// 5. Register the uri/id input pairs and the metric pubUri/id pair in the pub/sub
//    map (psmap) so metrics knows where where to put input data when it's received
/**
 * Inserts into the mdo with some cleanup
 * 1. Initialize the metric value with the supplied `value`
 * 2. Conform `inputs` URIs to the form { uri: "/frag0/.../fragn-1", id: "fragn"}
 * 3. Replace input list in entry with an object where the fully qualified URI is used as
 *    the key. The order of `inputs` stays the same as insertion order when used later
 * 4. Initialize input values in the metric entry with the supplied `inputValue`
 * 5. Register the uri/id input pairs and the metric pubUri/id pair in the pub/sub
 *    map (psmap) so metrics knows where where to put input data when it's received
 * @param {object} m metric command
 * @param {string} pubUri uri metric is published on
 * @param {*} value value to insert
 * @param {object} entry object that holds meta data for entry
 * @param {*} inputValue original inputs
 */
function insertEntry(m, pubUri, value, entry, inputValue) {
    /* eslint-disable no-param-reassign */
    entry.value = value;
    entry.inputs = {};
    m.inputs.forEach((input) => {
        // Initialize inputs
        let uri = input.uri.startsWith('/') ? input.uri : `/${input.uri}`; // make sure there's a leading slash
        if (uri.endsWith('/')) uri = uri.substring(0, uri.length - 1); // make sure there's no trailing slash
        let id = input.id.startsWith('/') ? input.id.substring(1) : input.id; // make sure there's no leading slash
        if (id.endsWith('/')) id = id.substring(0, id.length - 1); // make sure there's no trailing slash
        const fullUri = `${uri}/${id}`;
        entry.inputs[fullUri] = inputValue;
        /* eslint-enable no-param-reassign */
        // TODO: Clean up and initialize outputs similarly
        // Configure pub/set map
        if (!(uri in psmap)) {
            psmap[uri] = {};
        }
        // If another metric already listened to this input, push
        // this metric to that list, otherwise create the entry in psmap
        if (psmap[uri][id]) {
            psmap[uri][id].push({ uri: pubUri, id: m.id });
        } else psmap[uri][id] = [{ uri: pubUri, id: m.id }];
    });
    mdo[pubUri][m.id] = entry;
}

// TODO: resolve FIMS subscription rules with fims_server, see https://github.com/flexgen-power/metrics/issues/19
/**
 * Subscribes to non shadowed URIs
 * @param {string} uri uri to subscribe
 */
function subscribeShadow(uri) {
    // Subscribe to each uri, but only if it's not already shadowed by another
    // Also clean out shadowed uris at each step
    // e.g. /assets/ess would shadow /assets/ess/ess_1
    if (!subscribes.find((sub) => utils.UriIsRootOfUri(uri, sub))) {
        subscribes.push(uri);
        subscribes = subscribes.filter(
          (sub) => sub === uri || !utils.UriIsRootOfUri(sub, uri)
        );
    }
}

/**
 * Subscribes to all URIs
 */
function processSubscribes() {
    Object.keys(psmap).forEach(subscribeShadow);
    Object.keys(mdo).forEach(subscribeShadow);
    subscribes.forEach(fims.subscribeTo);
}

// requestValueFromStorage() sends a `get` to the `storage` module to get the last
// published value for a particular metric.
// 1. Subscribe to the URI of interest (metrics hasn't subscribed to anything else yet)
// 2. fims.send a message to storage
// 3. fims.receiveWithTimeout for 1s to wait for the response
// 4. If the response is received and is clean, call the `callback` function
//    supplied as an argument back in the initialization phase
// 5. If no response received, call the callback with the `no response` string
// 6. Unsubscribe from that URI
/**
 * Sends a `get` to the `storage` module to get the last
 * published value for a particular metric.
 * 1. Subscribe to the URI of interest (metrics hasn't subscribed to anything else yet)
 * 2. fims.send a message to storage
 * 3. fims.receiveWithTimeout for 1s to wait for the response
 * 4. If the response is received and is clean, call the `callback` function
 *    supplied as an argument back in the initialization phase
 * 5. If no response received, call the callback with the `no response` string
 * 6. Unsubscribe from that URI
 * @param {string} key key used to pull from object
 * @param {string} pubUri uri metric is published on
 * @param {function} callback sets values based on if default or not
 */
function requestValueFromStorage(key, pubUri, callback) {
    const replyto = `${pubUri}/${key}`;
    let lastValue = 'no response received';
    fims.subscribeTo(replyto);
    fims.send({
        method: 'get', uri: `/storage${replyto}`, replyto, body: {},
    });
    fims.receiveWithTimeout(1000000, (msg) => { // microseconds, equals 1 second
        if (msg) {
            if (msg.method === 'set' && msg.uri === replyto) {
                const response = msg.body[key];
                if (response !== undefined) {
                    // eslint-disable-next-line no-prototype-builtins
                    lastValue = response.value.hasOwnProperty('value') ? response.value.value : response.value;
                } else {
                    lastValue = 0;
                }
            } else {
                console.error('ERROR: metrics/requestValueFromStorage: Received an unexpected message back from FIMS');
            }
            callback(lastValue);
        }
    });
    setTimeout(() => {
        // this is a failsafe in case we don't get a response from storage.
        // if more than the receiveWithTimeout time elapses and lastValue
        // has not changed then we'll send the "no response" to the callback
        if (lastValue === 'no response received') {
            console.error('ERROR: metrics/requestValueFromStorage: Did not receive response from storage');
            callback(lastValue);
        }
    }, 1010); // milliseconds, equals 1.01 seconds
    try {
        fims.unsubscribeFrom(replyto);
    } catch (err) {
        console.log(`Error unsubscribing in metrics/requestValueFromStorage: ${err}`);
    }
}

// ---- END INITIALIZATION ----

// ---- WORKER FUNCTIONS ----

// processFims() takes a FIMS message, figures out if it should pay attention,
// if it's valid, then finds the values and pushes them to the inputs of metrics.
// 1. First check if `msg` is defined, then clean up the URI if needed
// 2. For pubs and sets
// 2.a. Call `searchForValues` which digs through the FIMS body for values,
//      which then calls `insertMdo` to push the inputs to certain metrics
// 2.b. One more check and call to `insertMdo` deals with sets that have totally
//      bare bodies like {42.0}
// 2.c. Reply to the set if it has a replyto
// 3. For gets, look through the master data object for data metrics has
// 3.a. If the message URI exactly matches a pubUri in mdo, return the whole body
//      as if it were a pub
// 3.b. If the message URI is more qualified than the pubUri, dig through the
//      ids in mdo and return just that value
// 4. For dels, call `resetMetric` using the same logic flow as `get`
/**
 * Process fims recieves
 * @param {object} msg recieved object
 */
function processFims(msg) {
    if (msg) {
        let cleanUri = msg.uri;
        // ensure clean /frag1/frag2/frag3 style
        cleanUri = cleanUri.startsWith('/') ? cleanUri : `/${cleanUri}`;
        cleanUri = cleanUri.endsWith('/') ? cleanUri.substring(0, cleanUri.length - 1) : cleanUri;
        if (msg.method === 'pub' || msg.method === 'set') {
            // go through psmap and insert values if any metrics care about them
            const value = searchForValues(cleanUri, msg.body);
            // This check handles case where a naked body pub/set comes directly into
            // a register, like {uri:/assets/ess/ess_1/voltage, body:150}
            if (value === false || value === 0 || value) {
                const slashIndex = cleanUri.lastIndexOf('/');
                insertMdo(cleanUri.substring(0, slashIndex),
                    cleanUri.substring(slashIndex + 1), value);
            }
            // TODO: This set reply may be overly broad since while the set will be for
            // a URI that metrics cares about (from the subscribes), it may be for a value
            // that metrics is not responsible for
            if (msg.method === 'set' && msg.replyTo) {
                fims.send({
                    method: 'set', uri: msg.replyto, replyto: null, body: msg.body,
                });
            }
        } else if (msg.method === 'get' && msg.replyto) {
            // find metric in mdo and set back out
            Object.keys(mdo).forEach((pubUri) => {
                const body = mdo[pubUri];
                if (cleanUri === pubUri) {
                    fims.send({
                        method: 'set', uri: msg.replyto, replyto: null, body: prepareBody(pubUri),
                    });
                } else if (cleanUri.startsWith(pubUri)) {
                    const id = cleanUri.substring(pubUri.length + 1);
                    if (body[id]) {
                        fims.send({
                            method: 'set', uri: msg.replyto, replyto: null, body: prepareBody(pubUri, id),
                        });
                    }
                }
            });
        } else if (msg.method === 'del') {
            // find metric in mdo and reset state, only valid for certain ops
            Object.keys(mdo).forEach((pubUri) => {
                const body = mdo[pubUri];
                if (cleanUri === pubUri) {
                    Object.keys(body).forEach((id) => {
                        resetMetric(pubUri, id);
                    });
                    fims.send({
                        method: 'set', uri: msg.replyto, replyto: null, body: prepareBody(pubUri),
                    });
                } else if (cleanUri.startsWith(pubUri)) {
                    const id = cleanUri.substring(pubUri.length + 1);
                    if (body[id]) {
                        resetMetric(pubUri, id);
                        fims.send({
                            method: 'set', uri: msg.replyto, replyto: null, body: prepareBody(pubUri, id),
                        });
                    }
                }
            });
        }
    }
}

// prepareBody() prepares each entry in the master data object to be put out on FIMS
// 1. When both pubUri and id are supplied, prepare the single metric requested
// 2. When only pubUri is supplied, prepare each metric in that pubUri, combine to one body
// 3. When neither are supplied (debug only) gather all metrics data into a single body
/**
 * prepares each entry in the master data object to be put out on FIMS
 * 1. When both pubUri and id are supplied, prepare the single metric requested
 * 2. When only pubUri is supplied, prepare each metric in that pubUri, combine to one body
 * 3. When neither are supplied (debug only) gather all metrics data into a single body
 * @param {string} pubUri uri metric is published on
 * @param {string} id id of entry
 * @returns {object} body object
 */
function prepareBody(pubUri, id) {
    const body = {};
    if (pubUri && id) {
        return mdo[pubUri][id].naked ? nakedBody(mdo[pubUri][id]) : clothedBody(mdo[pubUri][id]);
    } if (pubUri && !id) {
        /* eslint-disable no-shadow */
        Object.keys(mdo[pubUri]).forEach((id) => {
            body[id] = mdo[pubUri][id].naked ? nakedBody(mdo[pubUri][id])
                : clothedBody(mdo[pubUri][id]);
        });
    } else {
        // This shouldn't run in normal operation, but just blast out
        // everything if this is called without pubUri or id
        Object.keys(mdo).forEach((pubUri) => {
            Object.keys(mdo[pubUri]).forEach((id) => {
                /* eslint-enable no-shadow */
                body[pubUri][id] = mdo[pubUri][id].naked ? nakedBody(mdo[pubUri][id])
                    : clothedBody(mdo[pubUri][id]);
            });
        });
    }
    return body;
}

// clothedBody() returns a body that only has the most relevant data for a FIMS message
// notably missing metrics-specific fields like outputs, param, and naked since they
// may gum up the works
/**
 * Returns a body that only has the most relevant data for a FIMS message
 * notably missing metrics-specific fields like outputs, param, and naked since they
 * may gum up the works
 * @param {object} obj original body object
 * @returns {object} same object with keys added
 */
function clothedBody(obj) {
    // TODO: decide if `outputs` should be published in clothed bodies
    const keys = ['name', 'ui_type', 'type', 'options', 'inputs', 'value', 'operation'];
    // .map creates an array of single key objects matching the string array above
    // .reduce packs those back into a full object for return
    return keys.map((k) => (k in obj ? { [k]: obj[k] } : {}))
        .reduce((keyobj, v) => Object.assign(keyobj, v), {});
}

// nakedBody() just returns the value of the metric for simple {"key":value} messages
/**
 * Returns the value of the metric for simple {"key":value} messages
 * @param {object} obj original body object
 * @returns {*} value
 */
function nakedBody(obj) {
    return obj.value;
}

// resetMetric() does just that, sets the value to 0 and empties the state object
// for a metric that has state
/**
 * sets the value to 0 and empties the state object for a metric that has state
 * @param {string} pubUri uri metric is published on
 * @param {string} id id of metric
 * @returns {boolean} true if metric has state
 */
function resetMetric(pubUri, id) {
    if (mdo[pubUri][id].state) {
        // TODO: If the use case comes up, we may need to see if we can reset
        // the value to the initialized
        mdo[pubUri][id].value = 0;
        mdo[pubUri][id].state = {};
        return true;
    }
    return false;
}

// updateAndPubMetrics() is called periodically to update, then pub all the metrics
// 1. Go through each pubUri, then metric in the master data object
// 2. Update them
// 3. Publish everything on pubUri
// 4. Send the published message through processFims() so that metrics that chain
//    off each other can get their inputs updated, since FIMS server does not send
//    you back your own pub.
/**
 * called periodically to update and then pub all metrics
 * 1. Go through each pubUri, then metric in the master data object
 * 2. Update them
 * 3. Publish everything on pubUri
 * 4. Send the published message through processFims() so that metrics that chain
 *    off each other can get their inputs updated, since FIMS server does not send
 *    you back your own pub.
 */
function updateAndPubMetrics() {
    // Go across mdo and run update
    Object.keys(mdo).forEach((pubUri) => {
        Object.keys(mdo[pubUri]).forEach((id) => {
            updateMetric(pubUri, id);
        });
        const msg = {
            method: 'pub', uri: pubUri, replyto: null, body: prepareBody(pubUri), username: null,
        };
        processFims(msg);
        fims.send(msg);
    });
}

// updateMetric() feeds the metric's input value list to the specified operation,
// along with the state and param objects.
// * While `inputs` in the master data object has URI and ID information, only
//   the values of those inputs are passed to the operation
// * Functions in javascript are first class, meaning they can be stored as variables
//   and called, as you see in comp[entry.operation](...) where comp is the object
//   exported by computations.js
/**
 * Feeds the metric's input value list to the specified operation,
 * along with the state and param objects.
 * * While `inputs` in the master data object has URI and ID information, only
 *    the values of those inputs are passed to the operation
 * * Functions in javascript are first class, meaning they can be stored as variables
 *    and called, as you see in comp[entry.operation](...) where comp is the object
 *    exported by computations.js
 * @param {string} pubUri uri metric is published on
 * @param {string} id id of metric
 */
function updateMetric(pubUri, id) {
    const entry = mdo[pubUri][id];
    const inputs = Object.values(entry.inputs);
    if (comp[entry.operation]) {
        // TODO: Have entry.state get assigned here and returned from computations
        // so we don't have to modify state indirectly in the function call. This
        // is the more functional programming approach
        entry.value = comp[entry.operation](inputs, entry.state, entry.param);
        mdo[pubUri][id] = entry;
        if (nextMdoWrite < Date.now()) {
            nextMdoWrite = Date.now() + 10000;
            const mdoAndPsmapData = {
                mdo,
                psmap,
                publishUrisHash,
            };
            try {
                fs.writeFileSync(mdoJsonPathAndFile, JSON.stringify(mdoAndPsmapData));
            } catch (err) {
                console.log(`+++++ ERROR: metrics mdo file could not be written: ${err}\nThis will not impact the function of the metrics module but it will prevent metrics from reloading its state in the event of a restart. This error should be corrected; check that metrics can write to its json config directory\n`);
            }
        }
    } else throw new Error(`Computation ${entry.operation} not found`);
}

// searchForValues() deals with FIMS message bodies, searching down the object tree
// for the register ID and the corresponding value, whether the body be naked or clothed,
// one level or many levels deep. It is a recursive function to deal with the unknown
// depth of the object tree
// 1. Check if the body is a JS object, but specifically not an array (which is just a
//    JS object with numbers as keys)
// 1.a. Iterate over the keys of that body object
// 1.b. If the key is 'value', stop! you've found a clothed body value, return to higher
//      level call of searchForValues() or back to processFims()
// 1.c. Otherwise, dig down further in that object (recursive call)
// 1.d. If the recursive call to searchForValues() returns a value from 1.b. or 2., insert
//      that into the master data object
// 2. If the body is not an object or array, you've got a boolean, number or string, return it
/**
 * Deals with FIMS message bodies, searching down the object tree
 * for the register ID and the corresponding value, whether the body be naked or clothed,
 * one level or many levels deep. It is a recursive function to deal with the unknown
 * depth of the object tree
 * 1. Check if the body is a JS object, but specifically not an array (which is just a
 *    JS object with numbers as keys)
 * 1.a. Iterate over the keys of that body object
 * 1.b. If the key is 'value', stop! you've found a clothed body value, return to higher
 *      level call of searchForValues() or back to processFims()
 * 1.c. Otherwise, dig down further in that object (recursive call)
 * 1.d. If the recursive call to searchForValues() returns a value from 1.b. or 2., insert
 *      that into the master data object
 * 2. If the body is not an object or array, you've got a boolean, number or string, return it
 * @param {string} uri uri to insert on
 * @param {object} body body to search on
 * @returns {object} returns body unless not an object
 */
function searchForValues(uri, body) {
    // Currently only ingests object or value bodies, no naked arrays
    if (typeof (body) === 'object' && !Array.isArray(body)) {
        // Object.keys(body).forEach((key) => {
        // <--- in pre-linting, this was `for (const key in body) {`
        const theKeysArray = Object.keys(body);
        for (let i = 0; i < theKeysArray.length; i += 1) {
            const key = theKeysArray[i];
            // Found deepest level, behind the "value" key, return that value at that key in body
            if (key === 'value') {
                return body[key];
                // Still need to dig, recurse down
            }
            const value = searchForValues(`${uri}/${key}`, body[key]);
            // When value is true, the last call found the deepest level
            // When value is false, the deepest level was found and handled in later calls
            if (value === false || value === 0 || value) insertMdo(uri, key, value);
        }
    } else {
        // Found deepest level, value is naked, return the body
        return body;
    }
    return null;
}

// insertMdo() puts the passed value into the inputs array for all metrics that
// care about it, based on uri and key
// 1. Find list metrics in `psmap` that care about this input
// 2. Iterate on those metrics and put the value in its place
// 3. Call processOutputs() since the inputs updated
// 4. Because we want metrics with `param.setOutputOnUpdate`
//    to set their outputs at every publish, regardless
//    of whether their inputs are active, we use an `else`
//    statement to catch them. If a URI and key is not in
//    the psmap, it may be in the mdo. So, we check to see
//    if it's in the mdo and if it has param.setOutputOnUpdate
//    set to true. If so, we send it to processOutputs.
//    This forces an update-and-pub even when the input has
//    not been updated.
/**
 * Puts the passed value into the inputs array for all metrics that
 * care about it, based on uri and key
 * 1. Find list metrics in `psmap` that care about this input
 * 2. Iterate on those metrics and put the value in its place
 * 3. Call processOutputs() since the inputs updated
 * 4. Because we want metrics with `param.setOutputOnUpdate`
 *    to set their outputs at every publish, regardless
 *    of whether their inputs are active, we use an `else`
 *    statement to catch them. If a URI and key is not in
 *    the psmap, it may be in the mdo. So, we check to see
 *    if it's in the mdo and if it has param.setOutputOnUpdate
 *    set to true. If so, we send it to processOutputs.
 *    This forces an update-and-pub even when the input has
 *    not been updated.
 * @param {string} uri uri to insert on
 * @param {string} key key to insert on
 * @param {*} value value to insert
 */
function insertMdo(uri, key, value) {
    const metrics = psmap[uri] ? psmap[uri][key] : null;
    if (metrics) {
        metrics.forEach((metric) => {
            const pubUri = metric.uri;
            const { id } = metric;
            mdo[pubUri][id].inputs[`${uri}/${key}`] = value;
            processOutputs(pubUri, id);
        });
    } else if (mdo[uri] && mdo[uri][key] && mdo[uri][key].param
        && mdo[uri][key].param.setOutputOnUpdate) {
        processOutputs(uri, key);
    }
}

// processOutputs() deals with sending `set`s to the outputs defined in the
// config if the value changed.
// 1. Check if the metric has outputs
// 2. Initialize state if it doesn't already have it
// 3. Call updateMetric (the same as called on the publish timer)
// 4. If the new value is different from the last value, send the value as
//    `set` to each of the outputs
// 5. Mark the current value as lastValue for the next iteration
/**
 * Deals with sending `set`s to the outputs defined in the
 * config if the value changed.
 * 1. Check if the metric has outputs
 * 2. Initialize state if it doesn't already have it
 * 3. Call updateMetric (the same as called on the publish timer)
 * 4. If the new value is different from the last value, send the value as
 *    `set` to each of the outputs
 * 5. Mark the current value as lastValue for the next iteration
 * @param {string} pubUri uri key
 * @param {*} id id key
 */
function processOutputs(pubUri, id) {
    const entry = mdo[pubUri][id];
    /* eslint-disable no-prototype-builtins */
    if (entry.hasOwnProperty('outputs')) {
        if (!entry.hasOwnProperty('state')) {
            entry.state = {};
            entry.state.lastValue = null;
        } else if (!entry.state.hasOwnProperty('lastValue')) {
            /* eslint-enable no-prototype-builtins */
            entry.state.lastValue = null;
        }
        updateMetric(pubUri, id);
        const { value } = entry;
        const { lastValue } = entry.state;
        if ((entry.param && entry.param.setOutputOnUpdate) || lastValue !== value) {
            entry.outputs.forEach((output) => {
                const body = output.naked ? value : { value };
                if (output.uri.includes('/ui/') && Object.keys(body)[0] === 'value') {
                    // we send naked bodies to the UI the same way that components
                    // send them. Without the key "value". The key is the ID.
                    fims.send({
                        method: 'set',
                        uri: output.uri,
                        replyto: null,
                        body: JSON.stringify({ [output.id]: body.value }),
                        username: null,
                    });
                } else {
                    fims.send({
                        method: 'set',
                        uri: `${output.uri}/${output.id}`,
                        replyto: null,
                        body,
                        username: null,
                    });
                }
            });
            entry.state.lastValue = value;
        }
    }
}

// ---- END WORKER FUNCTIONS ----

// ---- SET UP AND RUN ----

/**
 * Function to calculate sleep time
 * @param {number} milliseconds amount of time to sleep
 */
function sleep(milliseconds) {
    const start = new Date().getTime();
    for (let i = 0; i < 1e7; i += 1) {
        if ((new Date().getTime() - start) > milliseconds) {
            break;
        }
    }
}

if (processArgv3 === 'test') {
    console.log(mdo);
    console.log(psmap);
    // eslint-disable-next-line global-require, import/no-dynamic-require
    const ex = require(path.join(configPath, './examples.js'));
    Object.keys(ex).forEach((k) => {
        // console.log(k);
        const x = ex[k];
        console.log(x.comment);
        processFims(x);
        updateAndPubMetrics();
        x.id.forEach((y) => console.log(mdo[x.pubUri][y]));
        sleep(200);
    });
}

// ---- START publish and listen setup ----

/* eslint-disable no-param-reassign */
// PUBLISH
const publishBeat = {};
/**
 * Publish heartbeat for metrics
 * @param {function} func recursive callback
 * @param {number} delay delay time in milliseconds
 */
function publishRateBeat(func, delay) {
    if (!publishBeat.started) {
        publishBeat.func = func;
        publishBeat.delay = delay;
        publishBeat.startTime = Date.now();
        publishBeat.target = delay;
        publishBeat.started = true;
        publishBeat.count = 1;
        setTimeout(publishRateBeat, delay);
    } else {
        const elapsed = Date.now() - publishBeat.startTime;
        const adjust = publishBeat.target - elapsed;
        publishBeat.count += 1;
        publishBeat.func(publishBeat.count);
        publishBeat.target += publishBeat.delay;
        setTimeout(publishRateBeat, publishBeat.delay + adjust);
    }
}

publishRateBeat(() => {
    updateAndPubMetrics();
}, publishRate);

// FIMS LISTEN
const listenBeat = {};
/**
 * Fims listen heartbeat for metrics
 * @param {function} func recursive callback
 * @param {number} delay delay time in milliseconds 
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
    fims.receiveWithTimeout(500, processFims);
}, listenRate);

/* eslint-enable no-param-reassign */
// ---- END publish and listen setup ----

subscribes.forEach((x) => { fims.subscribeTo(x); });


function getSubscribes() {
    return subscribes.sort();
}

function setSubscribes(newSubscribes) {
    subscribes = newSubscribes;
}

module.exports = {
    configureMdo,
    subscribeShadow,
    getSubscribes,
    setSubscribes,
};