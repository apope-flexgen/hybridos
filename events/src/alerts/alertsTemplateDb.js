/* eslint-disable */ /* TODO remove this */
const mongoose = require('mongoose');
const { alertTemplateSchema } = require('./alertSchema');

const AlertTemplate = mongoose.model('alertTemplates', alertTemplateSchema);
/**
 * Query events.alerts module
 * @module alertsDb
 */
module.exports = {
    AlertTemplate,

    async getAlertTemplates() {
        try {
            const q = AlertTemplate.find();
            const docs = await q.exec();
            const docsMapped = docs.map((doc) => ({
                id: doc.id,
                from: doc.from,
                minWidth: doc.minWidth,
                step: doc.step,
                to: doc.to,
                token: doc.token,
                type: doc.type,
                ...(doc.list && doc.list.length ? {list: doc.list} : {}),
                ...(doc.range && doc.range.length ? {range: doc.range} : {}),
            }));
            docsMapped.sort((a, b) => a.id < b.id ? -1 : 1)
            return docsMapped;
        } catch (err) {
            console.log(`mongoDB query error: ${err}`);
            return [];
        }
    },

    async upsertAlertTemplates(body) {
        try {
            await Promise.all(body.map(async (template) => {
                await AlertTemplate.findOneAndUpdate(
                    { id: template.id },
                    template,
                    { upsert: true },
                );
            }));
            return null;
        } catch (err) {
            console.log(`mongoDB query error: ${err}`);
            return 'Error updating Alert Template entry in database';
        }
    },

    async deleteAlertTemplates(idsToDelete) {
        try {
            await AlertTemplate.deleteMany({
                id: { $in: idsToDelete}
            });
            return null;
        } catch (err) {
            console.log(`mongoDB query error: ${err}`);
            return 'Error updating Alert Template entry in database';
        }
    }
};
