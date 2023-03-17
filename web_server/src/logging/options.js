const dirname = '/var/log/flexgen/web_server/';
const maxLogLevel = process.env.NODE_ENV === 'development' ? 'debug' : 'info';

var options = {
    file: {
      level: maxLogLevel,
      filename: `web_server.log`,
      dirname,
      handleExceptions: true,
      json: true,
      colorize: true,
    },
    console: {
      level: 'debug',
      handleExceptions: true,
      json: true,
      colorize: true,
    },
  };

  module.exports = options;