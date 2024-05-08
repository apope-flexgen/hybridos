module.exports = {
  env: {
    node: true,
  },
  extends: [
    'eslint:recommended',
    'plugin:@typescript-eslint/recommended',
    'prettier',
    'airbnb',
    'airbnb/hooks',
    'airbnb-typescript',
  ],
  parser: '@typescript-eslint/parser',
  parserOptions: {
    project: ['./tsconfig.eslint.json'],
    tsconfigRootDir: __dirname,
    sourceType: 'module',
  },
  plugins: ['@typescript-eslint', 'autofix', 'react-hooks'],
  root: true,
  ignorePatterns: [
    '.eslintrc.js',
    '.eslintrc-dev.js',
    'public/*',
    'cypress.config.ts',
    'src/**/*.spec.ts',
    'src/**/*.spec.tsx',
    'vite.config.ts',
  ],
  // ignorePatterns: ['.eslintrc.js', 'support/*', 'e2e/*']
  rules: {
    'react/react-in-jsx-scope': 'off',
    'max-len': [
      'error',
      {
        code: 100,
        tabWidth: 2,
        ignoreComments: true,
        ignoreUrls: true,
        ignoreStrings: true,
        ignoreTemplateLiterals: true,
      },
    ],
    'react/prop-types': 'off',
    'react/require-default-props': 'off',
    'arrow-body-style': ['error', 'as-needed'],
    'react/self-closing-comp': ['error', { component: true, html: true }],
    'autofix/no-unused-vars': [
      'error',
      {
        argsIgnorePattern: '^_',
        ignoreRestSiblings: true,
        destructuredArrayIgnorePattern: '^_',
      },
    ],
    'import/order': [
      'error',
      {
        groups: ['builtin', 'external', 'parent', 'sibling', 'index', 'object', 'type'],
        pathGroups: [
          {
            pattern: '@/**/**',
            group: 'parent',
            position: 'before',
          },
        ],
        alphabetize: { order: 'asc' },
      },
    ],
    'no-restricted-imports': [
      'error',
      {
        patterns: ['../'],
      },
    ],
    'react-hooks/exhaustive-deps': 'warn',
    'react/function-component-definition': [
      2,
      {
        namedComponents: 'arrow-function',
        unnamedComponents: 'arrow-function',
      },
    ],
    'react/jsx-props-no-spreading': 'off',
    'react/jsx-no-useless-fragment': 'warn',
    'max-lines': [
      'error',
      {
        max: 120,
        skipBlankLines: true,
        skipComments: true,
      },
    ],
    'import/prefer-default-export': 'off',
  },
};
