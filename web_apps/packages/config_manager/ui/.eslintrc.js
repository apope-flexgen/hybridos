module.exports = {
    parser: '@typescript-eslint/parser',
    // root: true,
    parserOptions: {
        project: 'tsconfig.json',
        tsconfigRootDir: __dirname,
        sourceType: 'module',
    },
    extends: ['../../shared/.eslintrc.js'],
    env: {
        es6: true,
        browser: true,
    },
    plugins: [
        'react',
        // 'react-hooks'
        '@typescript-eslint',
    ],
    globals: {},
    ignorePatterns: ['.eslintrc.js', '.eslintrc-dev.js', 'craco.config.js', 'public/*'],
    rules: {
        // react hooks
        // 'react-hooks/rules-of-hooks': 'error', // Checks rules of Hooks
        // 'react-hooks/exhaustive-deps': 'warn', // Checks effect dependencies
        // react
        'react/prop-types': 'off',
        'react/jsx-filename-extension': [1, { extensions: ['.js', '.jsx', 'tsx'] }],
        'react/no-find-dom-node': 'off',
        'jsx-quotes': ['error', 'prefer-single'],
        'react/no-array-index-key': 'off',
        // one class per file, even if it's stateless func
        'react/no-multi-comp': [2, { ignoreStateless: true }],
        'react/jsx-sort-props': 'error',
        // a11y
        // 'jsx-a11y/label-has-for': 'warn',
        // 'jsx-a11y/anchor-is-valid': 'warn',
    },
}
