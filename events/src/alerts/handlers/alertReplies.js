const { genericReply } = require('./utils');

const expirationMs = 2000;
// Handler to allow for queueing and handling of requests that need to be confirmed before completed.
// At the moment, this is only needed for alert management POST's.
const alertReplyhandler = {
    entries: [],
};

function addReplyListener(uri, onReply, onTimeout) {
    alertReplyhandler.entries.push({
        uri,
        onReply,
        onTimeout,
        expiration: new Date().getTime() + expirationMs,
    });
}

// Should be called regularly. Checks handlers for expired entries and invokes onTimeout().
function checkAlertTimeouts() {
    for (const entry of alertReplyhandler.entries) {
        const now = new Date().getTime();
        if (now > entry.expiration) {
            console.debug(`Request with uri ${entry.uri} timed out`);
            entry.onTimeout();
            alertReplyhandler.entries = alertReplyhandler.entries.filter((x) => x.uri !== entry.uri);
        }
    }
}

async function handleReply(msg) {
    // Find relevant handler
    const listenerEntry = alertReplyhandler.entries.find((x) => x.uri === msg.uri);
    if (!listenerEntry) {
        genericReply(msg, 'Reply uri invalid or expired');
        return;
    }

    // invoke callback entry
    listenerEntry.onReply(msg);
    // pop callback entry
    alertReplyhandler.entries = alertReplyhandler.entries.filter((x) => x.uri !== msg.uri);

    genericReply(msg, null);
}

module.exports = {
    addReplyListener,
    checkAlertTimeouts,
    handleReply,
};
