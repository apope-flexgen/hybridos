const mongoose = require('mongoose');

const { Schema } = mongoose;

const passwordSettingSchema = new Schema({
    multi_factor_authentication: {
        type: Boolean,
        default: false,
    },
    password_expiration: {
        type: Boolean,
        default: false,
    },
    password_expiration_interval: {
        type: String,
        default: '8d',
        validate: (password_expiration_interval) => {
            if (password_expiration_interval.match(/^[0-9]{1,4}[dm]/) === null) {
                throw new Error('password expiration interval must use 1-4 digits and d or m (day or month)');
            }
        },
    },
    minimum_password_length: {
        type: Number,
        default: 5,
        min: 5,
        max: 96,
    },
    maximum_password_length: {
        type: Number,
        default: 128,
        min: 16,
        max: 1028,
    },
    password_regular_expression: {
        type: String,
        default: /^(?=.*[a-z])(?=.*[A-Z])(?=.*[0-9])(?=.*[!"#\$%&'\*\+,\.\/:;=\\?@\^`\|~])/
    },
    old_passwords: {
        type: Number,
        default: 0,
        min: 0,
        max: 50,
    },
})


const radiusSettingSchema = new Schema({
    is_enabled: {
        type: Boolean,
        default: false,
    },
    ip_address: {
        type: String,
        default: '127.0.0.1',
        validate: (ip_address) => {
            if (ip_address.match(/^(?:[0-9]{1,3}\.){3}[0-9]{1,3}$/) === null) {
                throw new Error('IP address must be valid');
            }
        }
    },
    port: {
        type: Number,
        default: 0000
    },
    secret_phrase: {
        type: String,
        default: ''
    },
    wait_time: {
        type: Number,
        default: 5000
    }
})

const siteAdminSchema = new Schema({
    version: {
        type: String
    },
    password: {
        type: passwordSettingSchema,
        default: () => ({})
    },
    radius: {
        type: radiusSettingSchema,
        default: () => ({})
    },
});

const SiteAdmin = mongoose.model('SiteAdmin', siteAdminSchema);

module.exports = SiteAdmin;