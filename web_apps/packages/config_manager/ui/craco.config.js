const path = require('path')
const fs = require('fs')
const eslintConfig = require('./.eslintrc-dev.js')
const cracoBabelLoader = require('craco-babel-loader')

// Handle relative paths to sibling packages
const appDirectory = fs.realpathSync(process.cwd())
const resolvePackage = (relativePath) => path.resolve(appDirectory, relativePath)

module.exports = {
    eslint: {
        configure: eslintConfig,
    },
    plugins: [
        {
            plugin: cracoBabelLoader,
            options: {
                includes: [resolvePackage('../../shared')],
            },
        },
    ],
    babel: {
        presets: [
            [
                '@babel/preset-env',
                {
                    bugfixes: true,
                    shippedProposals: true,
                    corejs: 3,
                    useBuiltIns: 'entry',
                },
            ],
            ['@babel/preset-react', { runtime: 'automatic' }],
        ],
        plugins: [
            '@babel/plugin-proposal-optional-chaining',
            '@babel/plugin-proposal-nullish-coalescing-operator',
        ],
    },
    webpack: {
        configure: (webpackConfig, { env, paths }) => {
            const scopePluginIndex = webpackConfig.resolve.plugins.findIndex(
                ({ constructor }) => constructor && constructor.name === 'ModuleScopePlugin'
            )
            webpackConfig.resolve.plugins.splice(scopePluginIndex, 1)
            webpackConfig.resolve.alias = {
                react: path.resolve('./node_modules/react'),
                shared: path.resolve('../../shared'),
            }
            return webpackConfig
        },
    },
}
