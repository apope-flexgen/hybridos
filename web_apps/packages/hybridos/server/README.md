# Nest web_server

## Prerequisites

### Vagrant Setup

1. Complete 'Provisioning a Development Environment (Vagrant)' instructions [here](https://flexgen.atlassian.net/wiki/spaces/DEVOPS/pages/1605633/Provisioning+a+Development+Environment+Vagrant) if you haven't already.
2. Complete 'Configuring HybridOS' instructions [here](https://flexgen.atlassian.net/wiki/spaces/DEVOPS/pages/6324237/Configuring+HybridOS) if you haven't already.
3. After Vagrant setup is complete, navigate to the `nest_web_server` project root directory and follow the instructions below for Config Setup, Building, Running and Testing.

### Local (non-VM) Setup

1. Clone repo: `git clone git@github.com:flexgen-power/nest_web_server.git`
2. After repo is cloned, navigate to the `nest_web_server` project root directory and follow the instructions below for Building, Running and Testing.

## Building

- The set of preconfigured build commands should work for most purposes, but more details on configuring your own build are given below
- Preconfigured commands (all commands begin with `npm run`, e.g. `npm run start:dev`):

  - "build:dev": build a dev webpack but do not autostart
  - "build:prod": build a prod-ready executable into ./executable/nest
  - "start:dev": build and run a dev webpack
  - "start:local": build and run a dev webpack with mocks for local dev
  - "start:prod": run the executabe at ./executable/nest (you must build:prod before you start:prod)

- Configuring your own build:
- Building is based on a few hyphenated flags (passed via NODE_ENV), currently:
  - "devTools": use dev tools, such as hot module reload
  - "start": autostart after build
  - "fims": use real fims
  - "use[Prod|Dev|LocalProd]": use production config | development config | local production config
  - "noTimeout": disable HTTP timeout for debugging
- For example, to run with dev tools, autostart, and the dev config, you would set `NODE_ENV=devTools-start-useDev`

- webpack.config.js also offers an aggregates table, translating one passed flag into several, so the following NODE_ENVs are equivalent:
  - "prod" = "fims-useProd"
  - "dev" = "devTools-start-fims-useDev"
  - "local" = "devTools-start-useDev"
  - "debug" = "useDev-fims"
- Aggregates define the most commonly used run modes and simplify the custom commands created in package.json

- To add a new build command in package.json, add a new key to the "scripts" object, then define your command as the associated value

  - use NODE_ENV to pass build flags
  - follow the webpack command format used in other commands to make sure the webpack uses the correct config

- When building, you should see a message that says: `##### IF YOU DON'T SEE THIS, YOUR WEBPACK IS NOT BUILDING CORRECTLY #####`

  - if you don't the webpack.config.js is not running, this is because either
    - you are not pointing webpackPath to webpack.config.js in your npm run command
    - webpack.config.js has an error - because of nest abstraction, this error will not throw, the config will just be bypassed
      - the most common error here is a missing import, run `npm i` when in doubt

- Building for use with HybridOS in Vagrant environment: `bash ./package_utility/build.sh`
- Common errors:
  - If errors related to `gnu` or `gcc` are presented in your VM, try running `scl enable devtoolset-9 bash`.

## Running

- dev: `npm run start:dev` OR `npm run build:dev` + `node ./dist/main.js`
- prod: `npm run build:prod` + `./executable/nest`
  - the localProd config is for debugging executable, if you want to build localProd config you will have to define your own build command
- HOS: if your version of HOS supports nest_web_server, you can use the build and run scripts

## Testing

- Unit tests: `npm run test`
- e2e tests: `npm run test:e2e`
- Integration tests: `npm run test:integration`
- All tests: `npm run test:all`

## OpenAPI Documenation

The REST API documention Swagger .json and .html file are located in `./openapi`

### Generating new OpenAPI documentation files

- Generate a new Swagger .json from code: `npm run start`
- Generate a new HTML file from Swagger .json:
  1. Ensure you have redoc installed: `npm install -g redoc-cli`
  2. Run `redoc-cli bundle -o ./openapi/index.html ./openapi/swagger-spec.json`

# HybridOS Web Server Authentication

## Features

- Access Token / Refresh Token
- Configurable Lifetime of Tokens (with maxAge)

## Guide
All non-public resources served by the web_server can only be accessed by clients with an `accessToken`. The `accessToken` timeout is configurable based on client needs, but there a maxAge enforced in code. The `accessToken` is returned from the server in the request response during `/login` or `/refresh_token`.

A `refreshToken` is also issued when `/login` or `/refresh_token` API calls are successful. The refreshToken timeout is also configurable based on client needs, but also has a maxAge enforced in code. The `refreshToken` is set a secure, httpOnly, sameSite cookie.

## Glossary

- web_server: HybridOS Web Server (nest + node)
- accessToken: [See oauth](https://www.oauth.com/oauth2-servers/access-tokens/access-token-response/)
- refreshToken: [See oauth](https://www.oauth.com/oauth2-servers/access-tokens/access-token-response/)

## Common errors (Vagrant)
- The server is failing to connect to mongodb
  - Is your mongodb upgraded from 3.4 to 6.0? If not, run `sudo bash upgrade-mongo.sh`
- zeromq/g++ error during `npm i`
  - zeromq looks for an upgraded g++ version that is not in our default Vagrant env. Run `scl enable devtoolset-11 bash` and try `npm i` again.

