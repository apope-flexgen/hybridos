/* eslint-disable @typescript-eslint/no-var-requires */
const path = require('path');

CONFIG_PATH = path.resolve(__dirname, '../../test/configs/test-config.json');
WEB_UI_JSON_CONFIG_PATH = path.resolve(__dirname, '../../test/configs/test-web_ui.json');
USE_TIMEOUT_INTERCEPTOR = false;
RADIUS_DICTIONARY_PATH = path.resolve(
  __dirname,
  '../../src/radius/dictionaries/dictionary.flexgen',
);

process.env.APP_SERVER_PORT = 443;
process.env.MONGO_DB_NAME = 'hybridos_authentication';
process.env.MONGO_URL = 'mongodb://localhost:27017';
process.env.HTTP_TIMEOUT = '10000';
process.env.REFRESH_TOKEN_TIMEOUT = '28800';
process.env.ACCESS_TOKEN_TIMEOUT = '180';
process.env.ACCESS_TOKEN_SECRET_FIMS_SOCKET = 'secret_fims_socket';
process.env.JWT_SECRET_KEY = 'secret_jwt_key';
process.env.ACCESS_TOKEN_SECRET_FIMS_SOCKET = 'secret_fims_socket';
process.env.JWT_SECRET_KEY_MFA = 'secret_mfa_key';
process.env.JWT_SECRET_KEY_PASSWORD_EXPIRATION = 'secret_passExp_key';
