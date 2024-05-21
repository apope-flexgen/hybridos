const fims = require('@flexgen/fims');

function genericReply(msg, err) {
    const body = {
        success: !err,
        message: err,
    };
    if (msg.replyto) {
        fims.send({
            method: 'set',
            uri: msg.replyto,
            replyto: null,
            body: JSON.stringify(body),
            username: null,
        });
    }
}

function pubToUI(message) {
    fims.send({
        method: 'pub',
        uri: '/events/alerts',
        replyto: null,
        body: JSON.stringify({
            message,
        }),
        username: null,
    });
}

/**
 *
 * Send an upsert to a go_metrics entry for a given alert.
 *
 * @param {object} alert An alert entry from mongo
 * @param {object} templates Global templates from mongo
 */
function updateGoMetricsConfig(alert, templates) {
    if (alert.deleted) {
        fims.send({
            method: 'del',
            uri: `/go_metrics/configuration/${alert.id}`,
            replyto: null,
            body: null,
            username: null,
        });
        return;
    }
    const inputs = (alert.aliases || []).reduce((acc, entry) => {
        acc[entry.alias.replaceAll(' ', '_')] = {
            // go_metrics requires a true/false string to be indicated as "bool" rather than "boolean"
            uri: entry.uri, type: entry.type == 'boolean' ? 'bool' : entry.type,
        };
        return acc;
    }, {});
    // send updated configuration to go_metrics
    const goMetricsObj = {
        inputs,
        templates,
        outputs: {
            [alert.id]: {
                uri: '/events/alerts',
                flags: ['clothed', 'post', 'sparse', 'flat', 'lonely', 'no_heartbeat'],
                attributes: { source: 'Alerts' },
            },
        },
        metrics: [],
    };

    goMetricsObj.metrics.push({
        id: alert.id,
        type: 'bool',
        outputs: alert.id,
        expression: alert.expression,
        alert: true,
        messages: alert.messages,
        enabled: alert.enabled,
    });

    // send new configuration to go_metrics
    fims.send({
        method: 'set',
        uri: `/go_metrics/configuration/${alert.id}`,
        replyto: null,
        body: JSON.stringify(goMetricsObj),
        username: null,
    });
}

// Map a list of items' organizationId field to "organization".
function orgIdsToNames(items, orgs) {
    return items.map((item) => {
        const { organizationId, ...restOfItem } = item;
        const orgEntry = orgs.find((org) => org.id === organizationId);
        if (!orgEntry || organizationId === 'deleted') {
            return {
                ...restOfItem,
                organization: 'Deleted',
            };
        }
        if (!orgEntry || !orgEntry.name) {
            console.error(`Org name not found for id ${organizationId}, defaulting to showing ID.`);
            return {
                ...restOfItem,
                organization: item.organizationId,
            };
        }
        return {
            ...restOfItem,
            organization: orgEntry.name,
        };
    });
}

// Map a list of items' organizationId field to "organization".
function orgNamesToIds(items, orgs) {
    return items.map((item) => {
        const { organization: orgName, ...restOfItem } = item;
        const orgEntry = orgs.find((org) => org.name === orgName);
        if (!orgEntry || orgName === 'Deleted') {
            return {
                ...restOfItem,
                organizationId: 'deleted',
            };
        }
        if (!orgEntry || !orgEntry.name) {
            console.error(`Org ID not found for id ${orgName}, defaulting to showing Name. This shouldn't happen.`);
            return {
                ...restOfItem,
                organizationId: orgName,
            };
        }
        return {
            ...restOfItem,
            organizationId: orgEntry.id,
        };
    });
}

module.exports = {
    genericReply,
    updateGoMetricsConfig,
    pubToUI,
    orgIdsToNames,
    orgNamesToIds,
};
