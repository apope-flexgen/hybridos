const fims = require('@flexgen/fims');
const { v4: uuidv4 } = require('uuid');
const { getAlertManagement, upsertAlertManagement } = require('../alertsDb');
const { validateAlert } = require('../validateAlert');
const {
    pubToUI, updateGoMetricsConfig, genericReply, orgIdsToNames: mapOrganizations,
    orgNamesToIds,
} = require('./utils');
const { getAlertOrganizations } = require('../alertsOrgDb');
const { getAlertTemplates, upsertAlertTemplates, deleteAlertTemplates } = require('../alertsTemplateDb');

module.exports = {
    /**
     * POST /events/alerts/management
     * UI is upserting a single alert management entry.
     * 1. Fetch relevant entry
     * 2. Combine with supplied entry, notably stuffing fields into history.
     * 3. Return to UI
     */
    async handlePostAlertsManagement(msg) {
        const validation = validateAlert(msg, {
            requiredGroups: [
                ['deadline', 'enabled', 'expression', 'organization', 'severity', 'title', 'messages'],
                ['id'],
            ],
            optional: ['deleted', 'aliases', 'templates'],
        });
        if (!validation.success) {
            if (msg.replyto) {
                fims.send({
                    method: 'set',
                    uri: msg.replyto,
                    replyto: null,
                    body: JSON.stringify(validation),
                    username: null,
                });
            }
            return;
        }
        // if only body.enabled
        // use THIS URI and shape req like { body: { enabled: true }}
        let { body } = msg;
        const orgs = await getAlertOrganizations();
        if (body.organization) {
            [body] = orgNamesToIds([body], orgs);
        }
        const alerts = await getAlertManagement(body);
        let old = {};
        if (body.id) {
            old = alerts[0];
            if (!old) {
                if (msg.replyto) {
                    fims.send({
                        method: 'set',
                        uri: msg.replyto,
                        replyto: null,
                        body: JSON.stringify({
                            message: `Error: no alert found for id [${body.id}]`,
                            success: false,
                        }),
                        username: null,
                    });
                }
                return;
            }
        }
        let newAlert = {
            // not settable from UI
            id: old.id || uuidv4(),
            createdAt: old.createdAt || new Date(),
            updatedAt: new Date(),
            version: old.id ? old.version + 1 : 0,
            lastTriggered: old.lastTriggered,

            // settable from UI
            aliases: body.aliases || old.aliases,
            messages: body.messages || old.messages,
            deadline: body.deadline || old.deadline,
            enabled: Object.keys(body).includes('enabled') ? body.enabled : old.enabled,
            deleted: (Object.keys(body).includes('deleted') ? body.deleted : old.deleted) || false,
            expression: body.expression || old.expression,
            organizationId: body.organizationId || old.organizationId,
            severity: body.severity || old.severity,
            title: body.title || old.title,
            resolved: old.resolved || [],
            unresolved: old.unresolved || [],

            // stuff former ui settable fields into history array except enabled
            history: old.id ? ([...(old.history || []), old]).map((his) => ({
                version: his.version,

                aliases: his.aliases,
                messages: his.messages,
                deadline: his.deadline,
                deleted: his.deleted,
                expression: his.expression,
                organizationId: his.organizationId,
                severity: his.severity,
                title: his.title,
                deprecatedAt: new Date().toISOString(),
            })) : [],
        };

        // if we're only requesting from among these keys, don't append more history or change updatedAt
        if (Object.keys(body).every((key) => [
            'id', 'enabled', 'deleted', 'templates',
        ].includes(key))) {
            newAlert = {
                ...newAlert,
                history: old.history,
                updatedAt: old.updatedAt,
                version: old.version,
            };
        }

        // join included templates with existing templates
        const oldTemplates = await getAlertTemplates();
        let templates = Object.keys(body).includes('templates') ? body.templates : oldTemplates;

        templates = templates.map((template) => ({
            // eslint-disable-next-line no-param-reassign
            ...template,
            id: template.id || uuidv4(),
        }));
        templates.sort((a, b) => (a.token < b.token ? -1 : 1));
        if (Object.keys(body).includes('templates')) {
            const templatesToDel = oldTemplates.reduce((acc, curr) => {
                if (!templates.find((tem) => tem.id === curr.id)) {
                    return [...acc, curr.id];
                }
                return acc;
            }, []);

            await upsertAlertTemplates(templates);
            await deleteAlertTemplates(templatesToDel);
        }
        templates.sort((a, b) => (a.token < b.token ? -1 : 1));
        updateGoMetricsConfig(newAlert, templates);

        // Upsert mongo
        const err = await upsertAlertManagement(newAlert);
        if (!err) {
            pubToUI('An alert configuration has been changed');
        }
        genericReply(msg, err);
    },

    /**
     * GET /events/alerts/management
     * UI is upserting a single alert management entry.
     * 1. Fetch entries from DB
     * 2. Reformat and return to UI
     *
     * Parameters:
     *  limit    {number} (number of results to return)
     *  page     {number} (which page in the results to return)
     *  sort     {number} (whether or not to invert the order)
     */
    async handleGetAlertsManagement(msg) {
        const validation = validateAlert(msg, {
            requiredGroups: [],
            optional: ['limit', 'page', 'sort'],
        });
        if (!validation.success) {
            if (msg.replyto) {
                fims.send({
                    method: 'set',
                    uri: msg.replyto,
                    replyto: null,
                    body: JSON.stringify(validation),
                    username: null,
                });
            }
            return;
        }
        // UI is retrieving alert management entries
        const { body } = msg;

        const orgs = await getAlertOrganizations();
        const alerts = await getAlertManagement(body);
        // We don't need the incidents and we hardcode only the fields we need
        let toReturn = alerts.map(
            (alt) => ({
                id: alt.id,
                updatedAt: alt.updatedAt,
                aliases: alt.aliases,
                messages: alt.messages,
                enabled: alt.enabled,
                deleted: alt.deleted,
                deadline: alt.deadline,
                expression: alt.expression,
                lastTriggered: alt.lastTriggered,
                organizationId: alt.organizationId,
                severity: alt.severity,
                title: alt.title,
                version: alt.version,

                history: mapOrganizations(
                    alt.history.map((his) => ({
                        aliases: his.aliases,
                        messages: his.messages,
                        deadline: his.deadline,
                        expression: his.expression,
                        lastTriggered: his.lastTriggered,
                        organizationId: his.organizationId,
                        severity: his.severity,
                        title: his.title,
                        version: his.version,
                    })),
                    orgs,
                ),
            }),
        );

        const templates = await getAlertTemplates();
        toReturn = {
            rows: mapOrganizations(toReturn, orgs),
            templates,
        };
        if (msg.replyto) {
            fims.send({
                method: 'set',
                uri: msg.replyto,
                replyto: null,
                body: JSON.stringify(toReturn),
                username: null,
            });
        }
    },
};
