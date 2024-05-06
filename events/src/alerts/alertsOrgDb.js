const mongoose = require('mongoose');
const { alertOrganizationSchema } = require('./alertSchema');

const AlertOrganization = mongoose.model('alertOrganizations', alertOrganizationSchema);
/**
 * Query events.alerts module
 * @module alertsDb
 */
module.exports = {
    AlertOrganization,

    async getAlertOrganizations() {
        try {
            const q = AlertOrganization.find();
            const docs = await q.exec();
            docs.sort((a, b) => ((a.name < b.name) ? -1 : 1));
            return docs;
        } catch (err) {
            console.log(`mongoDB GET Alert Organization query error: ${err}`);
            return [];
        }
    },

    async upsertAlertOrganizations(rows) {
        try {
            await Promise.all(rows.map(async (body) => {
                await AlertOrganization.findOneAndUpdate(
                    { id: body.id },
                    body,
                    { upsert: true },
                );
            }));
            return null;
        } catch (err) {
            console.log(`mongoDB query error: ${err}`);
            return 'Error updating Alert Organization entry in database';
        }
    },

    async removeAlertOrganizations(body) {
        try {
            await AlertOrganization.findOneAndDelete(
                { id: body.id },
            );
            return null;
        } catch (err) {
            console.log(`mongoDB query error: ${err}`);
            return 'Error deleting Alert Organization entry in database';
        }
    },
};
