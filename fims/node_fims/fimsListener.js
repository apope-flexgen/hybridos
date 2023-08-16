/* eslint-disable no-console */
/* eslint-disable new-cap */

// eslint-disable-next-line import/no-unresolved, import/no-absolute-path
const fimsNapi = require('/usr/local/lib/fims.node');

const fimsInstance = new fimsNapi.napi_fims();
const maxConnectionRetries = 5;

let subscribeList = [];
let clientProcessName = '';
let connectionRetries = 0;
/**
 * fims could close the fimsInstance connection at any time, for any reason.
 * The node process using fimsInstance would have no knowledge of the connection being dropped,
 * and may need to attempt to reconnect before exec of fims send/receive/subscribe,
 * so we track isConnectionClosedByClientProcess to know if a reconnect attempt is necessary.
 * If true, we do not attempt reconnection in processFimsCmd.
 */
let isConnectionClosedByClientProcess = true;

module.exports = {
    fimsInstance,

    connect(pname = 'Undefined node process') {
        if (fimsInstance.isConnected(pname)) {
            module.exports.closeConnection(pname);
        }

        const connected = fimsInstance.connect(pname);
        if (connected) {
            console.log(`fims connection opened by ${pname}.`);
            clientProcessName = pname;
            isConnectionClosedByClientProcess = false;
            connectionRetries = 0;
        } else {
            if (connectionRetries < maxConnectionRetries) {
                connectionRetries += 1;
                console.log(`${pname} retrying connect to fims: ${connectionRetries}/${maxConnectionRetries}`);
                module.exports.connect(pname);
                return;
            }
            console.log(`${pname} failed to connect to fims.`);
        }
    },

    receiveWithTimeout(timeout, callback) {
        return processFimsCmd(() => {
            const fimsMessage = new fimsNapi.napi_fims_message();
            fimsInstance.receiveTimeout(fimsMessage, timeout);
            let body = fimsMessage.getBody();
            if (fimsMessage.getMethod() !== 0 && fimsMessage.getUri() !== 0) {
                try {
                    body = JSON.parse(body);
                } catch (err) {
                    console.log('There was a problem in receiveWithTimeout: ', err);
                }

                const msg = {
                    method: fimsMessage.getMethod(),
                    uri: fimsMessage.getUri(),
                    replyto: fimsMessage.getReplyTo(),
                    body,
                    username: fimsMessage.getUsername(),
                };
                callback(msg);
                return true;
            }
            callback();
            return false;
        });
    },

    send(msg) {
        return processFimsCmd(() => {
            let { body } = msg;
            if (!('username' in msg)){
                msg.username = null;
            }
            if (typeof (body) !== 'string') {
                try {
                    body = JSON.stringify(body);
                } catch (err) {
                    console.log('There was a problem creating the FIMS message to send: ', err);
                    return false;
                }
            }
            try {
                const result = fimsInstance.send(msg.method, msg.uri, msg.replyto, body, msg.username);
                return result;
            } catch (err) {
                console.log('There was a problem sending to FIMS: ', err);
                return false;
            }
        });
    },

    subscribeToList(uriList) {
        return processFimsCmd(() => {
            return fimsInstance.subscribe(...uriList);
        });
    },

    subscribeTo(uri) {
        return processFimsCmd(() => {
            if (!subscribeList.includes(uri)) {
                // console.log('subscribeTo()...about to subscribe to uri_array:',subscribeList);
                let subscribed = fimsInstance.subscribe(...[...subscribeList, uri]);
                if (subscribed) {
                    console.log(`${clientProcessName} subscribe to ${uri} succeeded.`)
                    subscribeList.push(uri);
                    return true;
                } else {
                    console.log(`${clientProcessName} subscribe for ${uri} failed.`)
                    return false;
                }
            }
            console.log('subscribeTo()...already subscribed to uri ', uri, ' in ', subscribeList);
            return true;
        });
    },

    unsubscribeFrom(uri) {
        return processFimsCmd(() => {
            subscribeList = subscribeList.filter((x) => (x !== uri));
            // console.log('unsubscribeFrom()...about to unsubscribe from uri_array:',subscribeList);
            if (subscribeList.length) return fimsInstance.subscribe(...subscribeList);

            console.log("unsubscribeFrom()...can't subscribe to empty list");
            return false;
        });
    },

    closeConnection(pname = 'Undefined node process') {
        console.log(`${pname} closing fims connnection`);
        fimsInstance.close(pname);
        isConnectionClosedByClientProcess = !fimsInstance.isConnected(pname);
        if (isConnectionClosedByClientProcess) {
            console.log(`fims connection closed by ${pname}.`);
        } else {
            console.log(`${pname} fims connection failed to close.`);
        }
        return isConnectionClosedByClientProcess;
    },
};

function processFimsCmd(cmd) {
    let success;
    success = cmd()
    if (!success) {
        if (connectionRetries < maxConnectionRetries && isConnectionClosedByFims()) {
            console.log(`processFimsCmd: ${clientProcessName} attempting fims reconnect.`);
            module.exports.connect(clientProcessName);
            return cmd();
        }
    }
    return success
}

function isConnectionClosedByFims() {
    return !fimsInstance.isConnected(clientProcessName) && !isConnectionClosedByClientProcess;
}

['SIGINT', 'SIGTERM'].forEach((eventType) => {
    process.on(eventType, () => {
        console.log(`${clientProcessName} closing fims connection due to ${eventType}.`);
        module.exports.closeConnection(clientProcessName);
        throw Error(`${eventType} detected.`);
    });
});
