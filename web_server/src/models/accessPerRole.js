const mongoose = require('mongoose');
const path = require('path');
// eslint-disable-next-line import/no-unresolved
const Cryptr = require('cryptr');
require('dotenv').config({
    path: path.join(__dirname, '../../.env'),
});

const cryptr = new Cryptr(process.env.JWT_SECRET);

const { Schema } = mongoose;

const AccessPerRoleSchema = new Schema({
    permissions: {
        type: String,
        required: true,
    },
});

AccessPerRoleSchema.methods.doEncrypt = (objectToEncrypt) => cryptr
    .encrypt(JSON.stringify(objectToEncrypt));

AccessPerRoleSchema.methods.doDecrypt = (objectToDecrypt) => JSON
    .parse(cryptr.decrypt(objectToDecrypt));

const AccessPerRole = mongoose.model('AccessPerRole', AccessPerRoleSchema);

module.exports = AccessPerRole;
