const winston = require('winston');
const expressWinston = require("express-winston")
const transports = require("./transports")
const syslogFormat = require("./syslogFormat")

function expressLogger(){
    return expressWinston.logger({
    transports,
    format: syslogFormat,
    requestFilter(req, propName) {
        if (req.url.includes('&password=') && (propName === 'url' || propName === 'originalUrl' || propName === 'query')) {
            // !'#\$%&\*\+,\./:;\=\?@\^`|~
            const theRedactedURL = req.url.replace(/\&password=([A-Za-z0-9_!'#\$%&\*\+,\./:;\=\?@\^`|~]+)/g, '&password=*****');
            return theRedactedURL;
        }
        return req[propName];
    },
    meta: true, 
    msg: `HTTP {{req.method}} {{req.url}}`, 
    expressFormat: true, 
   
  });
}

module.exports = new(expressLogger);