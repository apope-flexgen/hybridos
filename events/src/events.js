/* eslint-disable no-console */
// eslint-disable-next-line import/no-unresolved
const fims = require('fims');
const { Event, eventsQuery, clearActiveEventQuery } = require('./eventsDb.js');

fims.connect('events');

fims.subscribeTo('/events');

/**
 * Processes fims messages
 * @param {object} msg message to process
 */
function processEvent(msg) {
    if (msg) {
        const { body } = msg;
        const { eventsQueryID } = body;
        // On GET, process time range, severity level.
        // Make request to Mongo, spit the result back to FIMS in reply.
        if (msg.method === 'get') {
            eventsQuery(body, (evt) => {
                if (eventsQueryID > '') {
                    // if there is an eventsQueryID then we add the eventsQueryID
                    // into the front of the object we are sending back
                    // TODO: linting wants to change the following "unnecessary quoting"
                    // but we should test it before committing the change. No time to
                    // test it at present - we'll do so next time other code changes are
                    // made
                    if (typeof evt === 'object') {
                        evt.unshift({ "eventsQueryID": eventsQueryID })
                    } else {
                        // if evt is not an object with mongo documents
                        // then it is a number - the count of documents in the query
                        // eslint-disable-next-line no-param-reassign
                        evt = [{ "eventsQueryID": eventsQueryID }, { "totalNumberInQuery": evt }]
                    }
                }
                fims.send({
                    method: 'set',
                    uri: msg.replyto,
                    replyto: null,
                    body: JSON.stringify(evt),
                    username: null
                });
            });

            // On POST, timestamp it (if not present), enter it into the database.
            // If 'assert' flag is true, also set 'active' flag
            // If 'clear' flag is true, go find and update the referenced event,
            // and add this event to the database
            // In the future, this will call external notification systems.
        } else if (msg.method === 'post') {
            if (!body.timestamp) body.timestamp = Date.now();
            if (body.assert) {
                body.active = true;
            }
            const evt = Event(body);
            evt.save((err, event) => {
                if (err) {
                    console.log('problem ', err);
                } else {
                    if (msg.replyto) {
                        fims.send({
                            method: 'set',
                            uri: msg.replyto,
                            replyto: null,
                            body: JSON.stringify(event),
                            username: null
                        });
                    }
                    if (body.clear) {
                        clearActiveEventQuery(event, (activeEvt) => {
                            fims.send({
                                method: 'set',
                                uri: msg.replyto,
                                replyto: null,
                                body: JSON.stringify(activeEvt),
                                username: null
                            });
                        });
                    }
                }
            });
        }
    }
    setTimeout(() => {
        fims.receiveWithTimeout(500, processEvent);
    }, 20);
}

fims.receiveWithTimeout(500, processEvent);
