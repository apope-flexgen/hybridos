/* eslint-disable no-console */
// eslint-disable-next-line prefer-object-spread
const bcrypt = require('bcryptjs');
const crypto = require('crypto');
const User = require('../models/user');
const { authenticator } = require("otplib");

authenticator.options = { crypto };
/**
 * Default username for the default user
 * @type {string}
 */
const theDefaultUsername = 'fgdefault';

/**
 * Default password for the default user
 * @type {string}
 */
const theDefaultPassword = `${theDefaultUsername}1A!`;

/**
 * Default key for the default user
 * @type {string}
 */
const theDefaultSharedKey = authenticator.generateSecret();
/**
 * Create's a default user if no user is present
 */
function createDefaultUser() {
    User.countDocuments({}, (err, count) => {
        if (err) console.log('createDefaultUser error:', err);
        if (count === 0) {
            bcrypt.hash(theDefaultPassword, 10)
                .then(theDefaultPasswordHash => {
                    console.log('\n+++++++ zero users in mongodb database, creating default user\n');
                    const defaultAdmin = new User({
                        username: theDefaultUsername, password: theDefaultPasswordHash, role: 'developer',
                        mfa_enabled: false, shared_key: theDefaultSharedKey
                    });
                    defaultAdmin.save((error, user) => {
                        if (error) {
                            console.log(`\nERROR: ${defaultAdmin.username} not saved. error: ${error}\n`);
                        }
                        console.log(`SUCCESS: ${user.username} saved to users collection.\n`);
                    });
                })
                .catch(e => {
                    console.log("ERROR with default user password hash creation")
                })
        } else {
            console.log(`\n+++++++ ${count} existing users in MongoDB database\n`);
        }
    });
}

/**
 * Removes default user after log in
 */
const removeDefaultUser = async () => {
    try {
        const user = await User.findOne({ username: theDefaultUsername });
        if (user) {
            // eslint-disable-next-line no-underscore-dangle
            const theUserID = user._id;
            User.deleteOne({ _id: theUserID })
                .then((result) => {
                    if (result.deletedCount !== 1) {
                        console.log(`ERROR deleting default user: ${result}`);
                    } else {
                        console.log('======= deleted default user');
                    }
                });
        }
    } catch (error) {
        throw new Error(error);
    }
};

module.exports.createDefaultUser = createDefaultUser;
module.exports.removeDefaultUser = removeDefaultUser;
