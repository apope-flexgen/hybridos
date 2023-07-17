/* eslint-disable @typescript-eslint/no-var-requires */
const path = require('path');

CONFIG_PATH = path.resolve(__dirname, '../../test/configs/test-config.json');
FIMS = require('../../src/fims/mocks/fims.stubs.ts');
WORKER_PATH = path.resolve(__dirname, '../../src/fims/mocks/mockWorker.ts');
USE_TIMEOUT_INTERCEPTOR = false;
WEB_UI_JSON_CONFIG_PATH = path.resolve(__dirname, '../../test/configs/test-web_ui.json');
RADIUS_DICTIONARY_PATH = path.resolve(__dirname, '../../src/radius/dictionaries/dictionary.flexgen');

// override process.argv[4] to point to test permissions.json
process.argv[4] = './test/permissions';
