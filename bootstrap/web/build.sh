# this script creates an executable for the convertUserModel.js

set -xe

npm ci --production
npm prune --production
node_modules/pkg/lib-es5/bin.js --targets node16-linux-x64 convertUserModel.node.js -o convertUserModel
