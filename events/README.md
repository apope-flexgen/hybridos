# Usage

1. Upon first checkout, `npm install` and then `npm link fims` (If you haven't already, go to the node_fims directory and follow the readme there)

2. You will have to have mongodb installed. Expecting 4.0.3 or later, probably from the Mongo website, not your distribution's repository manager

3. `npm run start-db` will run the `mongod` process, using a config file in `/etc/mongod.conf`. Default settings are fine in development. This has to be sudo, so get your password ready.

4. If you need test data in your database, run `node src/loadDatabase`, which will put 6 entries of differing severity and flags.

5. `npm run rest` will start `fims_server` (assuming you are running from a checkout of HybridOS and FIMS is .. from you), then the `events` script itself, and you're ready to go.

6. Refer to `examples.js` for how to format your FIMS messages to the events module, and expected returns

7. If you need to start fresh at any point, `node src/clearDatabase` will delete all database entries with a timestamp before the current time. You didn't emit any events in the future, did you?