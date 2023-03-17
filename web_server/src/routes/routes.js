// TODO: At some point in the future, and for the sake of using the
// correct method, the .post functions in this file should be changed
// to .puts here and in the /web_ui repo.

// this file verifies the login username and password using auth.js (in the
// 'passport.authenticate' call), signs/creates a JWT, pushes that JWT
// into the array of valid JWTs in authConfig.js, and returns res.json
// to the browser as a secure cookie holding the jwt for future authorization
const fimsApi = require('fims');
const express = require('express');
const passport = require('passport');
const jwt = require('jsonwebtoken');
const Cryptr = require('cryptr');
const { totp, authenticator } = require("otplib");
const crypto = require("crypto");
const  qrcode  = require("qrcode");
const {
    jwtOptions, signOptions, validJWTs, shortJWTIdentifierStartCharacter,
    shortJWTIdentifierEndCharacter,
} = require('../auth/authConfig');
const SiteAdmin = require("../models/siteAdmin");
const { passwordAge, editUser } = require('../database/database');
const logger = require('../logging/logger');
const AuditLogger = require("../logging/auditLogger");
const { removeDefaultUser } = require('../database/manageDefaultUser');
const {
    extractCookieFromReq, checkIfValidJWT
} = require('../auth/authConfig');
const router = express.Router();
const cryptr = new Cryptr(process.env.JWT_SECRET);
let auditLogger = new AuditLogger(fimsApi);

const { customLog } = require('../logging/consoleTools');
const User = require('../models/user');

authenticator.options = { crypto }

router.get('/:request', (req, res) => {
    // use this if we want to see how many requests we've made and how many are left:
    // logger.info(req.rateLimit);
    const theRequest = req.params.request;
    let status = 200;
    let message = null;
    if (theRequest !== 'favicon.ico') {
        status = 401;
        message = 'API: unauthorized';
    }
    // console.log(`.............unrestricted GET: ${theRequest}`);
    res.json({
        status,
        message,
    });
});

router.post('/login', async (req, res, callback) => {
    // customLog('POST /login', 'g', true);
    passport.authenticate('login', async (err, user, info) => {
        try {
            if (err || !user) {
                logger.warn(`ROUTES AUTH - login error: ${info.message}`);
                return res.json(info);
            }
            req.login(user, { session: false }, async (error) => {
                if (error) return callback(error);
                let site = await SiteAdmin.findOne({});
                if (!site) { 
                    site = new SiteAdmin({});
                    await site.save((error, result)=>{});
                }    
                
                let mfa = await mfaCount(site, user);
                let requiredAuth = buildRequiredAuth(site);
                let passExpStatus = await passwordAge(site, user.username);

                if ( passExpStatus && passExpStatus.hasExpired === true){
                    passExpStatus.hasExpired = passExpStatus.hasExpired;
                    passExpStatus.username = user.username;
                }

                const token = jwt.sign({ 
                    user: user.username, 
                    role: user.role, 
                    userAgent: 
                    req.headers['user-agent'] 
                    },  
                    process.env.JWT_SECRET,
                    { expiresIn: signOptions[user.role].expTime }
                );

                const openPasswordExpirationModal = (site.password.password_expiration && passExpStatus.hasExpired)
                const userState = {jwt: token, username: user.username, role: user.role, user_id: user._id }
                const user_state_crypto = cryptr.encrypt(JSON.stringify(userState))
                if (site.password.password_expiration){
                    // this means I have to check to see if password is expired
                    if (passExpStatus){
                        // user can be tested to see if password is expired
                        if (passExpStatus.hasExpired === true) { 
                           
                            return res.json({
                                username: passExpStatus.username, 
                                password_expired: openPasswordExpirationModal,
                                mfa:mfa,
                                role: user.role, 
                                requiredAuth: requiredAuth,
                                user_state_crypto
                            })
                        } else {
                            // password expiration is not needed, remove from requiredAuth 
                            let myIndex = requiredAuth.indexOf('password_expiration');
                            if (myIndex !== -1) {
                                requiredAuth.splice(myIndex, 1);
                            }
                            // make an object to feed into superAuthProcessResult
                            let responseBody = {requiredAuth, role: user.role};
                            superAuthProcessResult(responseBody, res, userState);
                        }
                    } else {
                        // return an error
                    }
                } else if (site.password.multi_factor_authentication){
                    mfaResponse(res, user, requiredAuth, user_state_crypto);
                } else {
                    logger.info(`ROUTES AUTH - client successfully authenticated, issuing JWT authorization. user: ${user.username}, \\
                    role: ${user.role}, host: ${req.headers.host}, user-agent: ${req.headers['user-agent']}`);
                    res.cookie('jwt', token, jwtOptions[user.role])
                    validJWTs.push(token);
                    logger.info(`ROUTES AUTH - issued JWT ${token.substring(shortJWTIdentifierStartCharacter, shortJWTIdentifierEndCharacter)} (unique characters from JWT (${parseInt(shortJWTIdentifierStartCharacter, 10) + 1}-${parseInt(shortJWTIdentifierEndCharacter, 10) + 1}))`);
                    removeDefaultUser();
                    
                    if (passExpStatus.hasExpired === false){  
                        return res.json({
                            role: user.role, 
                            password_expired: openPasswordExpirationModal,
                            mfa:mfa,
                            time_to_expire: passExpStatus.timeToExpire,
                            requiredAuth: []
                        })
                    } else {
                        const trackerData = {username: user.username, userrole: user.role, modified_field: "user login", modified_value: true}
                        auditLogger.send(trackerData);
                        return res.json({
                            role: user.role,
                            requiredAuth: [],
                        })
                    }
                }
                
            });
        } catch (error) {
            return callback(error);
        }
        return null;
    })(req, res, callback);
    return null;
});

router.post('/superauth', async (req, res, callback) => {
    
    const userData = JSON.parse(cryptr.decrypt(req.body.user_state_crypto))
    jwt.verify(userData.jwt, process.env.JWT_SECRET, (err)=>{
        if (!err){
            
            if (req.query.superauthtype === "password_expiration"){
               
                editUser(req.body, (error, response) => {
                    if (error) console.log(error);
                    if (response) {
                        let myIndex = req.body.requiredAuth.indexOf('password_expiration');
                        if (myIndex !== -1) {
                            req.body.requiredAuth.splice(myIndex, 1);
                        }
                        superAuthProcessResult(req.body, res, userData);
                      
                    }
                });
            
            } else if (req.query.superauthtype === "multi_factor_authentication"){
                let myIndex = req.body.requiredAuth.indexOf('multi_factor_authentication');
                if (myIndex !== -1) {
                    req.body.requiredAuth.splice(myIndex, 1);
                }
                // TAKE req.body.totp and compare it with totp saved in doc (not implemented yet)
                // this needs to throw error if not pass
                // get shared key userin userData.username
                User.findOne({_id: userData.user_id})
                .then(userDoc => {
                    
                    const totpData = req.body.totp;
                  
                    const isValid = authenticator.verify({secret: userDoc.shared_key, token: totpData})
                    if (!isValid){
                        res.status(401).json({error: 'Unauthorized'});
                        //return res.json({message: "401 Authentication Failed"});
                    } else {
                        // set boolean in mfa, try even if already true
                        userDoc.mfa_enabled = true;
                        userDoc.save()
                        .then(()=>{
                            superAuthProcessResult(req.body, res, userData);
                        })
                        .catch(e=>{
                            console.log(e);
                        })
                    }
                    
                    
                })
                .catch(e=>{
                    console.log(e);
                })
            }
           
        }
    })
});


const superAuthProcessResult = async (body, res, userData)=>{
 
    if (body.requiredAuth.length > 0 ){
        const user_state_crypto = cryptr.encrypt(JSON.stringify(userData));
        mfaResponse(res, userData, body.requiredAuth, user_state_crypto);
        
    } else {
        validJWTs.push(userData.jwt);
        logger.info(`ROUTES AUTH - issued JWT ${userData.jwt.substring(shortJWTIdentifierStartCharacter, shortJWTIdentifierEndCharacter)} (unique characters from JWT (${parseInt(shortJWTIdentifierStartCharacter, 10) + 1}-${parseInt(shortJWTIdentifierEndCharacter, 10) + 1}))`);
        
        res.cookie('jwt', userData.jwt, jwtOptions[body.role])
        
        removeDefaultUser();
        return res.json({ role: body.role,
                          requiredAuth: []
                        });
        
    }
}

function mfaResponse(res, user, requiredAuth, user_state_crypto){
    if (user && user.mfa_enabled){
        return res.json({
            username: user.username, 
            mfa:2,
            secret_key: "",
            role: user.role,
            requiredAuth: requiredAuth,
            user_state_crypto
        })
    } else {
        buildQrString(user.username, "flexgen", user.shared_key, (imageUrl)=>{
            return res.json({
                username: user.username, 
                mfa:1,
                secret_key: imageUrl,
                role: user.role,
                requiredAuth: requiredAuth,
                user_state_crypto
            })
        })
    }
}

function buildQrString(user, service, secret, callback){

    let newSec = secret;
    const otpauth = authenticator.keyuri(
        encodeURIComponent(user),
        encodeURIComponent(service),
        newSec
      ) + "&chld=H|0";
       
      qrcode.toDataURL(otpauth, (err, imageUrl) => {
        if (err) {
          console.log('Error with QR');
          return;
        }
        callback(imageUrl)
      });
}

function buildRequiredAuth(site){
    let requiredAuth = [];
    if(site.password.password_expiration === true) requiredAuth.push('password_expiration')
    if(site.password.multi_factor_authentication === true) requiredAuth.push('multi_factor_authentication')
    return requiredAuth;
}

const mfaCount = async (site, user) => {
    let mfa=0;
    if (site.password.multi_factor_authentication===true) mfa++;
    if (user) {
        if (user.mfa_enabled) mfa++;
        return mfa;
    }
}

module.exports = router;
