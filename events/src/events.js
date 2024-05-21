/* eslint-disable no-console */
// eslint-disable-next-line import/no-unresolved
const fims = require('@flexgen/fims');
const { Event, eventsQuery, clearActiveEventQuery } = require('./eventsDb');
const {
    handleGetAlertsManagement,
    handlePostAlertsManagement,
} = require('./alerts/handlers/alertManagement');
const {
    handleGetAlerts,
    handlePostAlerts,
    handleSetAlerts,
    handleInitAlerts,
} = require('./alerts/handlers/alertIncidents');
const {
    handleGetAlertOrganizations,
    handlePostAlertOrganizations,
    handleDeleteAlertOrganizations,
} = require('./alerts/handlers/alertOrganizations');

fims.connect('events');
fims.subscribeTo('/events');

/**
 * Processes fims messages
 * @param {object} msg message to process
 */
function processEvent(msg) {
    // schedule next invocation, every 20ms
    if (process.env.NODE_ENV !== 'test') {
        setTimeout(() => {
            fims.receiveWithTimeout(500, processEvent);
        }, 20);
    }

    if (!msg) { return; }

    const { body } = msg;
    const segments = msg.uri.split('/');

    // /events/alerts endpoints
    if (segments[2] === 'alerts') {
        if (segments[3] === 'management') {
            if (msg.method === 'get') {
                handleGetAlertsManagement(msg);
            } else if (msg.method === 'post') {
                handlePostAlertsManagement(msg);
            }
        } else if (segments[3] === 'organizations') {
            if (msg.method === 'get') {
                handleGetAlertOrganizations(msg);
            } else if (msg.method === 'post') {
                handlePostAlertOrganizations(msg);
            } else if (msg.method === 'del') {
                handleDeleteAlertOrganizations(msg);
            }
        } else if (msg.method === 'get') {
            handleGetAlerts(msg);
        } else if (msg.method === 'post') {
            handlePostAlerts(msg);
        } else if (msg.method === 'set') {
            handleSetAlerts(msg);
        }
        return;
    }

    // On GET, process time range, severity level.
    // Make request to Mongo, spit the result back to FIMS in reply.
    if (msg.method === 'get') {
        // GET /events
        // UI is retrieving events
        const { eventsQueryID } = body;
        eventsQuery(body, (evt) => {
            if (eventsQueryID > '') {
                // if there is an eventsQueryID then we add the eventsQueryID
                // into the front of the object we are sending back
                // TODO: linting wants to change the following "unnecessary quoting"
                // but we should test it before committing the change. No time to
                // test it at present - we'll do so next time other code changes are
                // made
                if (typeof evt === 'object') {
                    evt.unshift({ eventsQueryID });
                } else {
                    // if evt is not an object with mongo documents
                    // then it is a number - the count of documents in the query
                    // eslint-disable-next-line no-param-reassign
                    evt = [{ eventsQueryID }, { totalNumberInQuery: evt }];
                }
            }
            if (msg.replyto) {
                fims.send({
                    method: 'set',
                    uri: msg.replyto,
                    replyto: null,
                    body: JSON.stringify(evt),
                    username: null,
                });
            }
        });

        // On POST, timestamp it (if not present), enter it into the database.
        // If 'assert' flag is true, also set 'active' flag
        // If 'clear' flag is true, go find and update the referenced event,
        // and add this event to the database
        // In the future, this will call external notification systems.
    } else if (msg.method === 'post') {
        // POST /events
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
                        username: null,
                    });
                }
                if (body.clear) {
                    clearActiveEventQuery(event, (activeEvt) => {
                        fims.send({
                            method: 'set',
                            uri: msg.replyto,
                            replyto: null,
                            body: JSON.stringify(activeEvt),
                            username: null,
                        });
                    });
                }
            }
        });
    }
}

if (process.env.NODE_ENV === 'dev') {
    // setup swagger UI
    const swaggerUi = require('swagger-ui-express');
    const express = require('express');
    const docsApp = express();
    const swaggerDoc = require('./swagger.json');
    docsApp.use('/docs', swaggerUi.serve, swaggerUi.setup(swaggerDoc));
    const PORT = 3001;
    docsApp.listen(PORT, () => {
        // console.log('WIP docs server is available at http://localhost:3001/docs/');
    });
}

if (process.env.NODE_ENV !== 'test') {
    handleInitAlerts();
    fims.receiveWithTimeout(500, processEvent);
}

module.exports = {
    processEvent,
};
