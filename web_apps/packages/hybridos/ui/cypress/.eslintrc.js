module.exports = {
  parser: '@typescript-eslint/parser',
  parserOptions: {
    sourceType: 'module',
    project: ['./tsconfig.eslint.json'],
  },
  extends: [
    'eslint:recommended',
    'plugin:@typescript-eslint/recommended',
    'prettier',
    'airbnb',
    'airbnb/hooks',
    'airbnb-typescript',
  ],
  env: {
    es6: true,
    browser: true,
  },
  plugins: ['react-hooks', 'autofix', '@typescript-eslint'],
  globals: {},
  ignorePatterns: ['.eslintrc.js', 'support/*', 'e2e/*'],
};
