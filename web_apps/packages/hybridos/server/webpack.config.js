/* eslint-disable @typescript-eslint/no-var-requires */
const { RunScriptWebpackPlugin } = require('run-script-webpack-plugin');
const nodeExternals = require('webpack-node-externals');
const path = require('path');
const { configs } = require('eslint-plugin-prettier');

// Options for node_env:
// devTools - use dev tools, such as hot module reload
// start - autostart after build
// fims - use real fims
// use[Prod|Dev|LocalProd] - use production config | development config | local production config
//
// Aggregates: use one flag to set multiple options
// prod - use production config, use fims
// dev - use development config, use dev tools, autostart after build, use fims (this may change when fims is fully mocked)
// local - use development config, dev tools, autostart after build, use mocks (currently only mocking fims, other mocks should be added to local options)

const aggregates = {
  prod: ['fims', 'useProd'],
  dev: ['devTools', 'start', 'fims', 'useDev'],
  local: ['devTools', 'start', 'useDev'],
  debug: ['fims', 'useDev', 'noTimeout'],
};

module.exports = function (options, webpack) {
  const node_env = process.env.NODE_ENV.trim();
  const envArgs =
    node_env in aggregates ? aggregates[node_env] : node_env.split('-');

  console.log(
    "##### IF YOU DON'T SEE THIS, YOUR WEBPACK IS NOT BUILDING CORRECTLY #####",
  );

  // default config - we will always want these options, regardless of run mode
  const config = {
    ...options,
    target: 'node',
    plugins: [
      ...options.plugins,
      new webpack.optimize.LimitChunkCountPlugin({
        maxChunks: 1,
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

  config.plugins.push(
    new webpack.DefinePlugin({
      RADIUS_DICTIONARY_PATH: JSON.stringify(
        path.resolve(__dirname, 'src/radius/dictionaries/dictionary.flexgen'),
      ),
    }),
  );

  // use real or mock fims?
  if (envArgs.includes('fims')) {
    config.plugins.push(
      new webpack.ProvidePlugin({
        FIMS: 'fims',
      }),
      new webpack.DefinePlugin({
        WORKER_PATH: `"${path.resolve(
          __dirname,
          'src/fims/worker/fimsWorker.ts',
        )}"`,
      }),
    );
  } else {
    config.plugins.push(
      new webpack.ProvidePlugin({
        FIMS: path.resolve(__dirname, 'src/fims/mocks/fims.stubs.ts'),
      }),
      new webpack.DefinePlugin({
        WORKER_PATH: JSON.stringify(
          path.resolve(__dirname, 'src/fims/mocks/mockWorker.ts'),
        ),
      }),
    );
  }

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

  // autostart after build (for commands like start:dev)
  if (envArgs.includes('start')) {
    config.plugins.push(
      new RunScriptWebpackPlugin({
        name: options.output.filename,
        autoRestart: false,
      }),
    );
  }

  // use production or development settings
  if (envArgs.includes('useProd')) {
    config.plugins.push(
      new webpack.DefinePlugin({
        CONFIG_PATH: `"/usr/local/etc/config/web_server/web_server.json"`,
      }),
    );
  } else if (envArgs.includes('useLocalProd')) {
    config.plugins.push(
      new webpack.DefinePlugin({
        CONFIG_PATH: `"${path.resolve(__dirname, 'configs/localProd-config')}"`,
      }),
    );
  } else if (envArgs.includes('useDev')) {
    config.plugins.push(
      new webpack.DefinePlugin({
        CONFIG_PATH: JSON.stringify(
          path.resolve(__dirname, 'configs/dev-config'),
        ),
      }),
    );
  }

  config.plugins.push(
    new webpack.DefinePlugin({
      WEB_UI_JSON_CONFIG_PATH: JSON.stringify(
        path.resolve(__dirname, 'configs/web_ui.json'),
      ),
    }),
  );

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
