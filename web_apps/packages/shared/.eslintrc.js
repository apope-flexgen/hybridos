module.exports = {
  parser: '@typescript-eslint/parser',
  // root: true,
  parserOptions: {
    tsconfigRootDir: __dirname,
    sourceType: 'module',
    allowImportExportEverywhere: true,
  },
  extends: [
    // 'react-app',
    // 'react-app/jest',
    // '@react-native-community',
    // 'airbnb',
    // 'plugin:react/recommended',
    // 'plugin:jest/recommended', // https://github.com/jest-community/eslint-plugin-jest
    'prettier',
  ],
  plugins: [
    // 'no-loops',
    // 'react',
    // 'prefer-object-spread',
    // 'react-hooks'
  ],
  // settings: {
  //   'import/resolver': {
  //     webpack: {
  //       config: { resolve: aliases },
  //     },
  //   },
  // },
  globals: {},
  ignorePatterns: ['.eslintrc.js'],
  rules: {
    // overrides
    'no-plusplus': 'off',
    'no-use-before-define': 'off', // RN styles
    'no-console': 1,
    'no-debugger': 1,
    'no-unused-vars': 1,
    // react hooks
    // 'react-hooks/rules-of-hooks': 'error', // Checks rules of Hooks
    // 'react-hooks/exhaustive-deps': 'warn', // Checks effect dependencies
    // react
    // 'react/prop-types': 'off',
    // 'react/jsx-filename-extension': [1, { extensions: ['.js', '.jsx', 'tsx'] }],
    // 'react/no-find-dom-node': 'off',
    // 'jsx-quotes': ['error', 'prefer-single'],
    // 'react/no-array-index-key': 'off',
    // one class per file, even if it's stateless func
    // 'react/no-multi-comp': [2, { ignoreStateless: true }],
    // 'react/jsx-sort-props': 'error',
    // a11y
    // 'jsx-a11y/label-has-for': 'warn',
    // 'jsx-a11y/anchor-is-valid': 'warn',
    // code complexity
    'max-len': ['error', { code: 130 }],
    'max-lines': ['error', { max: 200 }],
    'func-names': ['error', 'never'],
    'max-depth': ['error', 4],
    // 'max-statements': ['error', 15],
    'max-nested-callbacks': ['error', 3],
    // code aesthetics & idioms
    semi: [2, 'never'],
    quotes: ['error', 'single'],
    'no-unexpected-multiline': 'error', // guards against ASI edge cases so !semi is safe
    // 'no-loops/no-loops': 2,
    'spaced-comment': ['error', 'always', { exceptions: ['/**'] }],
    'no-named-as-default': 0,
    'object-curly-spacing': ['error', 'always'],
    'comma-dangle': [
      'error',
      {
        arrays: 'only-multiline',
        objects: 'only-multiline',
        imports: 'only-multiline',
        exports: 'only-multiline',
        functions: 'only-multiline',
      },
    ],
    // 'import/first': ['error', 'never'],
    // 'import/extensions': ['error', 'never'],
    'import/no-extraneous-dependencies': 'off',
    'import/no-unresolved': 'off',
    'import/prefer-default-export': 'off',
    'no-return-assign': 'off',
    'class-methods-use-this': 'off',
    'function-paren-newline': 'off',
    'arrow-body-style': 'off',
    'no-param-reassign': 'off',
    'lines-between-class-members': ['error', 'never'],
  },
};
