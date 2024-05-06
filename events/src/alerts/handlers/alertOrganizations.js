const fims = require('@flexgen/fims');
const { v4: uuidv4 } = require('uuid');
const { validateAlert } = require('../validateAlert');
const { genericReply } = require('./utils');
const { getAlertOrganizations, upsertAlertOrganizations, removeAlertOrganizations } = require('../alertsOrgDb');
const { getAlertManagement, upsertAlertManagement } = require('../alertsDb');

module.exports = {
    async handleGetAlertOrganizations(msg) {
        const validation = validateAlert(msg, {
            requiredGroups: [],
            optional: [],
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
        const orgs = await getAlertOrganizations();
        const toReturn = orgs.map((org) => ({ id: org.id, name: org.name }));
        toReturn.sort((a, b) => ((a.name < b.name) ? -1 : 1));
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

    async handlePostAlertOrganizations(msg) {
        const validation = validateAlert(msg, {
            requiredGroups: [
                ['rows'],
            ],
            optional: [],
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
        const { body } = msg;
        if (!body.rows.length) {
            genericReply(msg, 'Invalid organization array supplied');
            return;
        }

        const err = await upsertAlertOrganizations(
            msg.body.rows.map((row) => ({
                id: row.id || uuidv4(),
                name: row.name,
            })).filter(
                // name is required
                (row) => (!!row.name),
            ),
        );
        genericReply(msg, err);
    },

    async handleDeleteAlertOrganizations(msg) {
        const validation = validateAlert(msg, {
            requiredGroups: [
                ['id'],
            ],
            optional: [],
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
        const { body } = msg;
        const orgs = await getAlertOrganizations();
        const org = orgs.find((x) => x.id === body.id);
        if (!org) {
            genericReply(msg, `Organization of id ${body.id} does not exist.`);
            return;
        }
        const alerts = await getAlertManagement({});
        const entriesToUpsert = alerts.reduce((acc, curr) => {
            // cascade soft delete all relevant alert configs
            if (curr.organizationId === org.id && curr.deleted === false) {
                return [
                    ...acc,
                    { id: curr.id, deleted: true, organizationId: 'deleted' },
                ];
            }
            // trim out any incidents that reference this org
            const unresolved = curr.unresolved.filter((x) => {
                const xOrgId = curr.history[x.version] && curr.history[x.version].organizationId;
                return xOrgId && xOrgId !== org.id;
            });
            const resolved = curr.resolved.filter((x) => {
                const xOrgId = curr.history[x.version] && curr.history[x.version].organizationId;
                return xOrgId && xOrgId !== org.id;
            });
            if (unresolved.length !== curr.unresolved.length || resolved.length !== curr.resolved.length) {
                return [
                    ...acc,
                    { id: curr.id, unresolved, resolved },
                ];
            }
            return acc;
        }, []);

        await Promise.all(entriesToUpsert.map((alt) => upsertAlertManagement(alt)));

        // hard delete organization
        await removeAlertOrganizations(body);

        // cascade delete all relevant alert management entries
        genericReply(msg, null);
    },
};
