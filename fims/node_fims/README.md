# Local Usage

1. From here, `sudo npm link` will create a symlink on your system

2. In the project you want to run, `npm link fims` will create the other half of the symlink into that project's `node_modules` folder

3. In that project, you can then `const fims = require('fims');`

# Project Usage

1. Assuming you have required `fims` above, your app should be able to connect to a locally running `fims_server`

2. Your first order of business should be to `fims.subscribeTo(<uri>)`, where `<uri>` is either the path to your module, or path to other modules you want to subscribe to. `subscribeTo()` can be called multiple times to subscribe to more than one URI.

3. `fims.unsubscribeFrom(<uri>)` will unsubscribe you from the the URI specified while leaving your other subscriptions intact

4. `fims.receiveWithTimeout(timeout,callback)` is how you actually read FIMS messages off the channel. A timeout of 500 (microseconds) is sufficient to get messages and not block your program too long