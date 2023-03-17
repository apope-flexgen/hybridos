const mongoose = require('mongoose');
// eslint-disable-next-line import/no-unresolved, import/no-absolute-path
const bcrypt = require('bcryptjs');
// const bcrypt = require('/usr/local/lib/bcrypt_lib.node');

const { Schema } = mongoose;

const UserSchema = new Schema({
    username: {
        type: String,
        required: true,
        unique: true,
        lowercase: true,
        trim: true,
        minlength: [5, 'username must be at least 5 characters'],
        maxlength: [25, 'username must be no more than 25 characters'],
        validate: (username) => {
            if (username.match(/^(?!.*__.*)(?!.*\.\..*)[a-z0-9_.]+$/) === null) {
                throw new Error('username may only include letters, numbers, periods, and underscores');
            }
        },
    },
    password: {
        type: String,
        required: true,
    },
    password_updated: {
        type: Date,
        required: false,
    },
    role: {
        type: String,
        required: true,
        enum: { values: ['user', 'admin', 'rest', 'developer', 'observer'], message: 'role may only be "user", "admin", "rest", "developer", or "observer"' },
    },
    old_passwords: {
        type: Array,
    },
    shared_key: {
        type: String,
        required: true,
    },
    mfa_enabled: {
        type: Boolean,
        required: true,
    },
    version: {
        type: String,
        required: true,
        default: '1.0.0',
    }
});

// we do NOT use an arrow function here because we need to preserve "this"
// eslint-disable-next-line func-names
UserSchema.methods.isValidPassword = function (password) {
    // UserSchema.methods.isValidPassword = (password) => {
    // Hashes the password sent by the user for login and checks if the hashed password
    // stored in the database matches the one sent. Returns true if it does else false.
    return bcrypt.compare(password, this.password)
        .then((result) => result);
};

const User = mongoose.model('User', UserSchema);

module.exports = User;
