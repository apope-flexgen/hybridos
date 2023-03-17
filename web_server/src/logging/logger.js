const winston = require('winston');
const syslogFormat = require('./syslogFormat')
const transports = require('./transports')

const maxLogLevel = process.env.NODE_ENV === 'development' ? 'debug' : 'info';

var logger = winston.createLogger({
    level: maxLogLevel,
    format: syslogFormat,
    transports: transports,
    exitOnError: false, 
});
  
module.exports = logger;
