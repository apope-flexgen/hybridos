/* eslint-disable no-console */
/* eslint-disable new-cap */

// eslint-disable-next-line import/no-unresolved, import/no-absolute-path
const fimsNapi = require('/usr/local/lib/fims.node');

const fimsInstance = new fimsNapi.napi_fims();
// This call to connect is for backward compatibility only, this can
// be removed in the future when the other node based modules connect
// with their own process names
fimsInstance.connect();
let subscribeList = [];

module.exports = {
    fimsInstance,

    connect(pname = 'Undefined node process') {
        if (fimsInstance.isConnected()) {
            fimsInstance.close();
        }
        fimsInstance.connect(pname);
    },

    receiveWithTimeout(timeout, callback) {
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
    },

    send(msg) {
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
    },

    subscribeToList(uriList) {
        return fimsInstance.subscribe(...uriList);
    },

    subscribeTo(uri) {
        if (!subscribeList.includes(uri)) {
            subscribeList.push(uri);
            // console.log('subscribeTo()...about to subscribe to uri_array:',subscribeList);
            return fimsInstance.subscribe(...subscribeList);
        }
        console.log('subscribeTo()...already subscribed to uri ', uri, ' in ', subscribeList);
        return true;
    },

    unsubscribeFrom(uri) {
        subscribeList = subscribeList.filter((x) => (x !== uri));
        // console.log('unsubscribeFrom()...about to unsubscribe from uri_array:',subscribeList);
        if (subscribeList.length) return fimsInstance.subscribe(...subscribeList);

        console.log("unsubscribeFrom()...can't subscribe to empty list");
        return false;
    },

    closeConnection() {
        console.log('closeConnection()...closing fims connnection');
        const result = fimsInstance.close();

        return result;
    },
};
