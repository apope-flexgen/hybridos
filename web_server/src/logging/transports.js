var winston = require('winston');
var options = require("./options");

var transports = [
    new winston.transports.File(options.file),
    //new winston.transports.Console(options.console)
  ];


module.exports = transports;