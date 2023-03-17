// this file handles login (which includes validating user in mongoDB)
// and authenticates other requests through the http basic or JwtCookieComboStrategy

const passport = require('passport');
const path = require('path');
const LocalStrategy = require('passport-local').Strategy;
const HTTPBasicStrategy = require('passport-http').BasicStrategy;
const JwtCookieComboStrategy = require('passport-jwt-cookiecombo');
const UserModel = require('../models/user');
const SiteAdmin = require('../models/siteAdmin');
const AuthRadius = require('./auth-radius');

const { customLog } = require('../logging/consoleTools');

require('dotenv').config({
    // TODO: if the following does not work, use `path.resolve('../.env')`
    path: path.join(__dirname, '../../.env'),
});

passport.use('login', new LocalStrategy({
    usernameField: 'username',
    passwordField: 'password'
}, async (username, password, done) => {

    let siteAdmin;
    try {
        siteAdmin = await SiteAdmin.findOne({});
        if (!siteAdmin) {
            return await authenticateLocalUser(username, password, done);
        }
    } catch (error) {
        return done(error);
    }

    const {
        is_enabled,
        ip_address,
        secret_phrase,
        port,
        wait_time
    } = siteAdmin.radius;

    if (is_enabled) {
        const authRadius = new AuthRadius(ip_address, secret_phrase, port, wait_time);
        authRadius.authenticate(username, password)
            .onAccept(async function (decodedPacket) {
                const roleAttr = authRadius.getAttributeRole(decodedPacket);
                let role = roleAttr ? roleAttr.toLowerCase() : '';
                // if (role !== 'admin' && role !== 'user') {
                if (!['admin', 'user', 'observer'].includes(role)) {
                    return done(null, false, { message: `Role ${roleAttr} is not valid for SSO` });
                }

                const user = new UserModel({ 'username': username, 'role': role });
                return done(null, user, { message: 'AUTH - login: logged in successfully' });
            })
            .onReject(async function (decodedPacket) {
                return await authenticateLocalUser(username, password, done);
            })
            .onError(async function (error) {
                return done(null, false, { message: error.message });
            })
            .onTimeout(async function () {
                return await authenticateLocalUser(username, password, done);
            });
    } else {
        return await authenticateLocalUser(username, password, done);
    }
}));

async function authenticateLocalUser(username, password, done) {
    let user;
    try {
        user = await UserModel.findOne({ 'username': username });
        if (!user) {
            return done(null, false, { message: 'AUTH - login: username and/or password incorrect' });
        }
    } catch (error) {
        return done(error);
    }

    const validate = await user.isValidPassword(password);
    if (!validate) {
        return done(null, false, { message: 'AUTH - login: username and/or password incorrect' });
    }
    return done(null, user, { message: 'AUTH - login: logged in successfully' });
}

passport.use('basic', new HTTPBasicStrategy(async (username, password, done) => {
    return await authenticateLocalUser(username, password, done);
}));

// eslint-disable-next-line max-len
passport.use(new JwtCookieComboStrategy({ secretOrPublicKey: process.env.JWT_SECRET }, (payload, done) => {
    done(null, payload);
}));
