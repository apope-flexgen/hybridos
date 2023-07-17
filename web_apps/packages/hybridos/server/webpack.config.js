/* eslint-disable @typescript-eslint/no-var-requires */
const { RunScriptWebpackPlugin } = require('run-script-webpack-plugin');
const nodeExternals = require('webpack-node-externals');
const Dotenv = require('dotenv-webpack');
const CopyWebpackPlugin = require('copy-webpack-plugin');

// Options for node_env:
// devTools - use dev tools, such as hot module reload
// start - autostart after build
// fims - use real fims
//
// Aggregates: use one flag to set multiple options
// prod - use fims
// dev - use dev tools, autostart after build, use fims (this may change when fims is fully mocked)
// local - dev tools, autostart after build, use mocks (currently only mocking fims, other mocks should be added to local options)

const aggregates = {
  prod: ['fims', 'useProd'],
  dev: ['devTools', 'start', 'fims'],
  local: ['devTools', 'start'],
  debug: ['fims', 'noTimeout'],
};

const WEB_UI_BUILD_PATH = '/usr/local/bin/web_ui/';
const WEB_UI_CONFIG_PATH = '/usr/local/etc/config/web_ui';
const WEB_SERVER_CONFIG_PATH = '/usr/local/etc/config/web_server';

module.exports = function (options, webpack) {
  const node_env = process.env.NODE_ENV.trim();
  const envArgs = node_env in aggregates ? aggregates[node_env] : node_env.split('-');

  console.log('##### IF YOU SEE THIS, YOUR WEBPACK IS BUILDING CORRECTLY #####');

  // default config - we will always want these options, regardless of run mode
  const config = {
    ...options,
    target: 'node',
    plugins: [
      ...options.plugins,
      new webpack.optimize.LimitChunkCountPlugin({
        maxChunks: 1,
      }),
      new Dotenv(),
      new CopyWebpackPlugin({
        patterns: [
          {
            from: 'src/fims/worker/fimsWorker.js',
            to: 'fims/worker/fimsWorker.js',
          },
          {
            from: 'src/fims/worker/fimsWorker_replyto.js',
            to: 'fims/worker/fimsWorker_replyto.js',
          },
          { from: 'src/radius/dictionaries', to: 'dictionaries' },
        ],
      }),
    ],
    module: {
      rules: [
        {
          test: /\.ts$/,
          loader: 'ts-loader',
          options: {
            compilerOptions: {
              module: 'esnext',
            },
          },
        },
      ],
    },
  };

  // dev toolkit, like HMR
  if (envArgs.includes('devTools')) {
    config.plugins.push(new webpack.HotModuleReplacementPlugin());
    config.entry = ['webpack/hot/poll?100', config.entry];
    config.externals = [
      nodeExternals({
        allowlist: ['webpack/hot/poll?100'],
      }),
    ];
  }

  //Added this check so that the paths can be set by process in prod
  if (node_env !== 'prod') {
    // autostart after build (for commands like start:dev)
    if (envArgs.includes('start')) {
      config.plugins.push(
        new RunScriptWebpackPlugin({
          name: options.output.filename,
          autoRestart: false,
          args: [WEB_UI_BUILD_PATH, WEB_UI_CONFIG_PATH, WEB_SERVER_CONFIG_PATH],
        }),
      );
    }

    config.plugins.push(
      new webpack.DefinePlugin({
        CONFIG_PATH: `"${WEB_SERVER_CONFIG_PATH}/web_server.json"`,
      }),
    );

    config.plugins.push(
      new webpack.DefinePlugin({
        WEB_UI_JSON_CONFIG_PATH: `"${WEB_UI_CONFIG_PATH}/web_ui.json"`,
      }),
    );
  }
  if (envArgs.includes('noTimeout')) {
    config.plugins.push(
      new webpack.DefinePlugin({
        USE_TIMEOUT_INTERCEPTOR: 'false',
      }),
    );
  } else {
    config.plugins.push(
      new webpack.DefinePlugin({
        USE_TIMEOUT_INTERCEPTOR: 'true',
      }),
    );
  }

  return config;
};
