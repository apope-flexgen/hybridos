/* eslint-disable no-console */
const path = require('path');
// eslint-disable-next-line import/no-unresolved
const Cryptr = require('cryptr');
const base32 = require('hi-base32');
const crypto = require('crypto');
const bcrypt = require('bcryptjs');
const { totp, authenticator } = require("otplib");
const { time } = require('console');
const User = require('../models/user');
const SiteAdmin = require("../models/siteAdmin");
const AccessPerRole = require('../models/accessPerRole');
const { requestWhitelist } = require('express-winston');
const { builtinModules } = require('module');
const AuthRadius = require('../auth/auth-radius');

require('dotenv').config({
    path: path.join(__dirname, '../../.env'),
});

const { customLog } = require('../logging/consoleTools');
const { create } = require('domain');

authenticator.options = { crypto };
const cryptr = new Cryptr(process.env.JWT_SECRET);

const validatePassword = async (password)=>{
    let site;
    site = await SiteAdmin.findOne({});
    if (!site) site = new SiteAdmin({});

    const {
        password_regular_expression,
        minimum_password_length,
        maximum_password_length
    } = site.password;

    let regexPure = password_regular_expression.slice(1,-1);
    const regexTester = new RegExp(regexPure)
    const minLength = (password.length > minimum_password_length);
    const maxLength = (password.length < maximum_password_length);
    const regex = regexTester.test(password);
    
    if (minLength && maxLength && regex){
        return await bcrypt.hash(password, 10);
    } else {
        return false;
    }
    
}
/**
 * Creates a user in the mongo database
 * @param {object} req request object
 * @param {function} callback post processing callback
 */
const createUser = async (req, callback) => {
    const { username, password } = req;
    const role = req.role || 'user';
    // need to get the siteadmin data, look at bools
    // if password_expiration true create oldpasswords doc, 
    // if multi_factor_authentication true, create mfa doc
    const site = await SiteAdmin.findOne({});
    try {
        const password_updated = Date.now();
        const checkForDuplicate = await User.findOne({ username });
        if (checkForDuplicate) {
            callback({ message: `user "${username}" already exists` }, null);
            return;
        }
        const password_hash = await validatePassword(password);
        if (!password_hash) {
            callback({ message: `password failed validation` }, null);
            return;
        }
        const shared_key = authenticator.generateSecret();
        const mfa_enabled = false;
        const old_passwords = [];

        const user = new User({ username, password: password_hash, role, password_updated, old_passwords, shared_key, mfa_enabled });

        await user.save((error, result) => {            
            callback(error, result);
        });
    } catch (error) {
        throw new Error(error);
    }
};

/**
 * Removes a user from the mongo database
 * @param {object} req request object
 * @param {function} callback post processing callback
 */
const removeUser = async (req, callback) => {
    const { username } = req;
    try {
        const user = await User.findOne({ username });
        if (!user) {
            callback({ message: `user "${username}" not found` }, null);
            return;
        }
        // eslint-disable-next-line no-underscore-dangle
        const theUserID = user._id;
        User.deleteOne({ _id: theUserID })
            .then((result) => {
                if (result.deletedCount !== 1) {
                    callback({ message: `error deleting user "${username}": ${result}` }, null);
                }
                return null;
            });
        callback(null, user);
    } catch (error) {
        throw new Error(error);
    }
};


/**
 * edits a user in the mongo database
 * @param {object} req request object
 * @param {function} callback post processing callback
 */
const editUser = async (req, callback) => {
    let site;
    site = await SiteAdmin.findOne({});
    if (!site) site = new SiteAdmin({});
    const { old_passwords } = site.password;
    if (req.username){
        try {
            let user = await User.findOne({ username: req.username });
            if (user) {
                if(req.resetmfa && site.password.multi_factor_authentication) {
                    let shared_key = authenticator.generateSecret();
                    await User.updateOne({_id: user._id}, {mfa_enabled: !req.resetmfa, shared_key: shared_key});
                }
                if ("password" in req){

                    const password_hash = await validatePassword(req.password);

                    if (!password_hash) {
                        callback({ message: `password failed validation` }, null);
                        return;
                    }
                    
                    let site;
                    site = await SiteAdmin.findOne({});

                    if (!site || (old_passwords === 0)) {
                        user.password = password_hash;
                    } else if (old_passwords > 0){
                        
                        const searchResult = await oldPasswords(user, req.password, old_passwords)
                        if (searchResult){
                            if (searchResult.hash_in_old_passwords === false){
                                
                                user.password_updated = Date.now();
                                user.password = password_hash;
                                
                            } else {

                                callback({ message: `that password has been used recently, please choose another` }, null);
                                return;

                            }
                        }
                    }
                }
                const editables = ["role"]
                for (item of editables) {
                    if (item in req){                
                        user[item] = req[item];
                    }
                }

                await user.save((error, result) => {
                    callback(error, result);
                }); 
            } else {
                callback({ message: `user "${req.username}" does not exist` }, null);
                return;
            }
        } catch (error) {
            throw new Error(error);
        }
    } else {
        throw new Error("EDIT USER FAILED: no username provided")
    }
};

const oldPasswords = async (user, password, oldPasswordsThreshold)=>{
    
    let resultObject = {hash_in_old_passwords: false};
    let oldPasswordArr = user.old_passwords;

    if (oldPasswordArr == null){ 
        oldPasswordArr = user.old_passwords = [];
    } else {
        
        let result = await oldPasswordsCompare(password, oldPasswordArr);
        let currResult = await currPasswordCompare(password, user.password);
        if (result || currResult){
            resultObject.hash_in_old_passwords = true;
        } else {
            // dequeue
            while (oldPasswordArr.length >= oldPasswordsThreshold){
                const _ = oldPasswordArr.shift();
            }
            // enqueue
            oldPasswordArr.push(user.password);
        }
        
    }
    return resultObject;
}

const currPasswordCompare = (password, currPassword) => {
    return new Promise(resolve => {
        bcrypt.compare(password, currPassword, (err, comparison) => {
            if (err) {
                resolve(false);
            } else {
                resolve(comparison);
            }
        })
    })
}

const oldPasswordsCompare = (password, oldPasswordList) =>{

    return new Promise(resolve=>{
        if (oldPasswordList.length === 0) resolve(false);
        for (let i=0; i < oldPasswordList.length; i++){
            bcrypt.compare(password, oldPasswordList[i], (err,comparison)=>{
                if (err){
                    resolve(false)
                } else {
                  if (comparison === true) {
                      resolve(true);
                  } else {
                      if (i === oldPasswordList.length-1){
                          resolve(false);
                      }
                  }
                }
            })
        }
    })
}

/**
 * read all users, optional query criterion
 * @param {object} query optional criterion for mongoose find 
 * @param {function} callback post processing callback
 */
 const readUsers = async (query=false, callback) => {

    try {
        let users;

        if (query){

          users = await User.find( query, '_id role username' ).exec();

        } else {

          users = await User.find({}, '_id role username');

        }

        callback(null, users);

    } catch (error) {

        callback(error, null);
        throw new Error(error);

    }
};


/**
 * Returns time diff until password expires, also simple bool stating if has expired
 * @param {String} timeInterval time to password expire in format hours, days i.e. '8h', '90d' 
 * @param {String} userName name of user to look up in order to check if password has expired
 * returning Object {hasExpired: boolean, timeToExpire: Date}
 */
const passwordAge = async (site, username) => {
    
    return new Promise(resolve=>{
        if (site){
            if ("password" in site && "password_expiration" in site.password){
                const timeInterval = site.password.password_expiration_interval;
                let splitTest = timeInterval.split(/([dhm])/, 2)

                if (splitTest.length === 2){

                    const cleanNumberString = splitTest[0];
                    const cleanNumber = parseInt(cleanNumberString);
                    let intervalMilliseconds = 0;

                    switch(splitTest[1]){
                        case "d":
                            intervalMilliseconds = cleanNumber * 86400000;
                            break;
                        case "h":
                            intervalMilliseconds = cleanNumber * 3600000;
                            break;
                        case "m":
                            intervalMilliseconds = cleanNumber * 60000;
                            break;
                        default: 
                            break;
                    }
                    try {
                        User.findOne({ username })
                        .then(user=>{
                            if (!user) {
                                resolve(false)
                            } else {
                            
                                let hasExpired;
                                let timeToExpire; 
                                let hasTimestamp = ("password_updated" in user)
                                
                                if (hasTimestamp){
                                    timeToExpire = Date.now() - Date.parse(user.password_updated);
                                    hasExpired = (timeToExpire > intervalMilliseconds)
                                } else {
                                    hasExpired = true;
                                    timeToExpire = 0;
                                }
                                resolve({username, hasExpired, timeToExpire})
                            }
                        })
                    } catch (error) {
                        resolve(false);
                    }
                } else {
                    resolve(false);
                }
            } else {
                resolve(false);
            }
        } else {
            resolve(false);
        }
    })
};

const pruneOldPasswordsArray = (requestOldPasswords) => {
    return new Promise(resolve=>{   
        User.find({})
        .then(allUserDocs=>{
            for (let i = 0; i < allUserDocs.length; i++){   
                if (allUserDocs[i].old_passwords.length > requestOldPasswords){
                    const toPrune = allUserDocs[i].old_passwords.length - requestOldPasswords;
                    allUserDocs[i].old_passwords.splice(0, toPrune);
                    allUserDocs[i].save()
                    .catch(e=>{
                        console.log(e);
                    })
                }  
                if (i === allUserDocs.length-1) resolve(true);
            }
        })
        .catch(e=>{
            console.log(e);
            resolve(true);
        })
    })
}

const passwordUpdateField = () => {
    
    return new Promise(resolve=>{
        
        User.find({})
        .then(allUsers=>{
            for (let i = 0; i < allUsers.length; i++){ 
                
                const hasPassUpdated = allUsers[i].password_updated;
                
                if (!hasPassUpdated){
                    
                    allUsers[i].password_updated = Date.now();
                    
                    allUsers[i].save()
                    .catch(e=>{
                        console.log(e);
                    })
                }  
                if (i === allUsers.length-1) resolve(true);
            }
        })
        .catch(e=>{
            console.log(e);
            resolve(true);
        })
    })
}

const radiusTest = async (radiusConfig, done) => {
    try {
        const {
            ip_address,
            secret_phrase,
            port,
            wait_time,
            _username,
            _password
        } = radiusConfig;

        const testRadius = new AuthRadius(ip_address, secret_phrase, port, wait_time);
        testRadius.authenticate(_username, _password)
            .onAccept(async function (decodedPacket) {
                // customLog(`radiusTest() onAccept: ${JSON.stringify(decodedPacket)}`, 'g');
                return done(null, decodedPacket);
            })
            .onReject(async function (decodedPacket) {
                // customLog(`radiusTest() onReject: ${JSON.stringify(decodedPacket)}`, 'b');
                return done(new Error(`Radius connection established but login was unsuccessful`), null);
            })
            .onError(async function (error) {
                return done(error, null);
            })
            .onTimeout(async function () {
                return done(new Error('Connection timed out'), null);
            });
    } catch (error) {
        customLog(`Error in radius_test: ${error}`, 'r');
        done(error, null);
    }
}

const siteAdmin = async (req, callback) => {
    let site;
    site = await SiteAdmin.findOne({});
    if (!site) site = new SiteAdmin({});

    const {
        old_passwords,
        password_expiration,
        multi_factor_authentication
    } = site.password;

    // Checks if old_passwords has changed and needs to be purged
    if (old_passwords !== req.password.old_passwords) {
        let requestOldPasswords = parseInt(req.password.old_passwords, 10);
        if (requestOldPasswords < site.password.old_passwords){
            await pruneOldPasswordsArray(requestOldPasswords);
        }
    }
    
    // Checks if password_expiration has changed, if so tags all users
    if (password_expiration !== req.password.password_expiration) {
        if (req.password.password_expiration === "true" || req.password.password_expiration === true){
            await passwordUpdateField();
        }
    }

    // Checks if MFA has changed, if so tag all users
    if (multi_factor_authentication !== req.password.multi_factor_authentication) {
        if (req.password.multi_factor_authentication === "true" || req.password.multi_factor_authentication === true){
            // for each user update mfa fields
            User.find({})
                .then(users => {
                    if (users && (users.length >= 1)){
                        for (let u = 0; u < users.length; u++){
                            let shared_key = authenticator.generateSecret();
                            let mfa_enabled = false;
                            User.updateOne({_id: users[u]._id}, {shared_key: shared_key, mfa_enabled: mfa_enabled});
                        }
                    }
                })
        }
    }

    for (const setting in req) {
        if (!setting.startsWith('_')) {
            for (const subSetting in req[setting]) {
                if (!subSetting.startsWith('_')) {
                    try {
                        if (typeof req[setting][subSetting] !== typeof site[setting][subSetting]) {
                            throw new Error(`Type Mismatch: ${setting}, ${subSetting}`);
                        } else {
                            site[setting][subSetting] = req[setting][subSetting];
                        }
                    } catch (error) {
                        callback(error, null);
                        return;
                    }
                }
            }
        }
    }

    await site.save((error, result) => {
        callback(error, result);
    }); 
};

const readAllSiteAdmin = async (req, callback) => {
    try {
        let site;
        site = await SiteAdmin.findOne({});
        if (!site) site = new SiteAdmin({});
        await site.save((error, result) => {
            callback(error, result);
        }); 
    } catch (e) {
        callback({error: `${e}`}, null)
    }
}

const mfaCompare = async (req, callback) => {
    // take TOTP key and do comparison with secret key for user 
    
}

/**
 * Decrypts permissions and reads them
 * @param {function} callback function that processes permissions
 */
function readAndDecryptPermissions(callback) {
    AccessPerRole.findOne({})
        .then((result) => {
            const theDecryptedPermissions = JSON.parse(cryptr.decrypt(result.permissions));
            if (!theDecryptedPermissions) {
                console.log('+++++ ERROR: permissions not found in authentication database');
                return;
            }
            callback(theDecryptedPermissions);
        });
}

/**
 * Saves and encrypts permissions, then deletes original from config
 * @param {string[]} thePermissionsArray stores permissions
 * @param {function} callback post processing callback
 */
const encryptAndWritePermissions = async (thePermissionsArray, callback) => {
    try {
        AccessPerRole.deleteMany({}, (error, result) => {
            console.log('  +++ previous permissions deleted from authentication database.', result);
            const theEncryptedPermissions = cryptr.encrypt(JSON.stringify(thePermissionsArray));
            const thePermissionsArrayToSave = new AccessPerRole({
                permissions:
                    theEncryptedPermissions,
            });
            thePermissionsArrayToSave.save()
                .then((result2) => {
                    // eslint-disable-next-line no-underscore-dangle
                    console.log('  +++ new permissions encrypted and added to authentication database. Record ID:', result2._id);
                    console.log('  +++ new permissions object length:', Object.keys(thePermissionsArray).length);
                    readAndDecryptPermissions((result3) => {
                        callback(result3);
                    });
                });
        });
    } catch (error) {
        throw new Error(error);
    }
};

module.exports = {
    createUser,
    removeUser,
    siteAdmin,
    editUser,
    passwordAge,
    readUsers,
    readAllSiteAdmin,
    mfaCompare,
    radiusTest,
    encryptAndWritePermissions,
    readAndDecryptPermissions
}