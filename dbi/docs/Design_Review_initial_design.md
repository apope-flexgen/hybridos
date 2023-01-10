# Design Review - dbi module

# REVISION NOTE: `dbi` will be a single database of collections and documents. Whereas this document correlates databases to HybridOS modules, the design has been revised to make collections correlate to databases.

## Goals

"As a SWD, I want to be able to `set` individual commands to a database to retrieve later." This module can, with minor modifications to the connection method, work with any NoSQL database. Currently we are using MongoDB.


## Affected Repos

`dbi` is the only repo directly affected. Any other module will be able to query a database through `dbi` by sending a properly formatted FIMS message.


## MongoDB Terminology

Most MongoDB terminology is at least very similar to other database terminology. That said, it's worth clarifying a few terms:

**database**
: A physical container for collections. Each database gets its own set of files on the file system. A single MongoDB server typically has multiple databases.

**collection**
: A grouping of MongoDB documents. A collection is the equivalent of an RDBMS table. A collection exists within a single database. Collections do not enforce a schema. Documents within a collection can have different fields. Typically, all documents in a collection have a similar or related purpose.

**document**
: A record in a MongoDB collection and the basic unit of data in MongoDB. **Documents are analogous to JSON objects** but exist in the database in a more type-rich format known as BSON.

**BSON**
: A serialization format used to store documents and make remote procedure calls in MongoDB. “BSON” is a portmanteau of the words “binary” and “JSON”. *Think of BSON as a binary representation of JSON (JavaScript Object Notation) documents.*

The entire MongoDB glossary is located at [Glossary - MongoDB Manual](https://docs.mongodb.com/manual/reference/glossary/)


## Approach

At its most basic, `dbi` reads and writes json objects to a NoSQL database (we are currently using MongoDB) in response to queries received via FIMS. As the feature set grows - and depending on our priorities - `dbi` could handle versioning and complete CRUD (create, read, update, delete) operations on any sort of NoSQL database.

### Flow of Operations

When `dbi` starts, it will create a server instance connection using the line `const databaseConnection = mongoose.createConnection(``mongodb://localhost:27017/dbi``, { useNewUrlParser: true, useUnifiedTopology: true });` and then it will create databases as-needed on this server instance.

When a FIMS message with the method `set` is received, an `updateOne` operation is used to update or "upsert" a document. This means that if the document already exists, it will be updated. If it does not already exist, it will be created. N.B. This MVP calls for handling a single document per collection. In a collection with only a single document, operations like `find` are moot because find is for finding documents within a collection that match criteria. In future revisions when `dbi` deals with more than one document per collection, a `find` operation will commonly occur before any updating or creation of a new document. At that time, separate operations like `create` and `updateMany` may also be added. This would be an internal change - fully backward-compatible, the user's experience will not change.

When a FIMS message with the method `get` is received, a `find` is performed and the result returned.

When a FIMS message with the method `del` is received, a `deleteOne` is performed and the result returned. Similar to operations with the `set` method, in future revisions a `find` operation will commonly occur before deleting.

The process control structure is managed by a consistent "heartbeat", the same as in `/metrics` and other repos:
```
// START FIMS listen setup
const listenBeat = {};
function listenRateBeat(func, delay) {
    if (!listenBeat.started) {
        listenBeat.func = func;
        listenBeat.delay = delay;
        listenBeat.startTime = Date.now();
        listenBeat.target = delay;
        listenBeat.started = true;
        listenBeat.count = 1;
        setTimeout(listenRateBeat, delay);
    } else {
        const elapsed = Date.now() - listenBeat.startTime;
        const adjust = listenBeat.target - elapsed;
        listenBeat.count += 1;
        listenBeat.func(listenBeat.count);
        listenBeat.target += listenBeat.delay;
        setTimeout(listenRateBeat, listenBeat.delay + adjust);
    }
}

listenRateBeat(() => {
    fims.receiveWithTimeout(500, processEvent)
}, listenRate);
```

To match the methods used in other HybridOS repos, there is no specific "tear down" sequence for `dbi`; if the process is ended or fails, it will simply re-establish connections when it starts again.

If `mongod` dies or the connection is lost, `.spec` and `.service` files will restart those processes. Another module that uses MongoDB is `/events`. /events does not use any special detection of MongodDB connection status, but we could certainly add status detection here (such as `connection.on('disconnected', () => {`) and initiate various functions if there is a need.


## Interface

The interface for `dbi` is a FIMS message. We will use a simple "set" operation as an example:
```
/usr/local/bin/fims/fims_send -m set -u /dbi/:database/:collection/:document '[body]'
```

Here's a breakdown of that message: The method is `set` which will create or update a document. The URI starts with `dbi` and then the database name. The database name will correspond directly with the module using the data (e.g., `site_controller` will use a database named `site_controller`). Following the database name is the collection name. A module can do whatever they wish with this, but will commonly have separate collections related to its operations such as `assets`. Within a collection, there is a single document (A single document for the MVP. Provisions for multiple documents can be added in future revisions). A document is analogous to a JSON object and for most intents and purposes can be treated as one. Additional URI segments can be added after the document name to "drill" down deeper to update a specific key/value pair within the document, e.g.:
```
/usr/local/bin/fims/fims_send -m pub -u /dbi/site_controller/assets/ess/nominal_frequency '40'
```
See [***Examples of Commands***](#examples-of-commands) below for more information.

After the URI is the body of the FIMS message. The body contains the data used to create or update the document at the endpoint described by the URI. The body can be a JSON object, an empty object, or a raw value or boolean. Once the FIMS message is parsed and the query submitted to the database, the database's response is returned to the module that initiated the query as a FIMS `set` when a `replyto` address is available.


## Configuration

When `dbi` starts, it will create a server instance connection using the line `const databaseConnection = mongoose.createConnection(``mongodb://localhost:27017/dbi``, { useNewUrlParser: true, useUnifiedTopology: true });` and then it will create databases as-needed on this server instance.


## Notes on Methods

Here is what `dbi` will do with FIMS messages of various methods:

* `set` - create or update a document, updates by REPLACING data at whatever level is indicated by the endpoint
* `get` - find and return a document
* `pub` - NOT USED in this module
* `post` - NOT TO BE IMPLEMENTED in MVP
* `del` - remove an existing document

Results of database queries are returned via FIMS as a `set` when a `replyto` address is available.


## Examples of Commands<a name="examples-of-commands"></a>

### Create Database
The creation of databases and collections are handled *within the context of a document's creation*. MongoDB does not create an empty space with a label on it, if you will. If you want a new database for, say, the `site_controller` module, MongoDB will automatically create that database the moment you create the first collection and document for it. Prior to the existence of the first document, MongoDB sees no reason to track the existance of an "idea" with no substance (documents). This is one significant difference between SQL and noSQL databases.

When MongoDB is presented with the creation or updating of a document, it creates the whole collection/document tree *if it doesn't already exist*. If a collection does already exist, then of course, it will simply add or update documents as needed. In other words, creating a document creates the structure that document needs, if that structure doesn't already exist. It's a little like Harry Potter's "Room of Requirement".

### Create or Update Document
```
Method: `set`
URI: `/dbi/:database/:collection/:document`
Body: `{key: value[, key: value...]}`
```
FIMS message example: `/usr/local/bin/fims/fims_send -m set -u /dbi/site_controller/assets/ess '{"nominal_voltage": 550.0,"nominal_frequency": 60.0,"start_value": 207,"stop_value": 207}'`


**EXAMPLE OF CREATING AND UPDATING A DOCUMENT**

If we walk through a few steps, we can see how this plays out. First we will create a new document in a new collection with the following FIMS message:
`/usr/local/bin/fims/fims_send -m set -u /dbi/site_controller/assets/ess/nominal_frequency '40'`
And get this result:
```
{
    "_id": ObjectId("6013368ea05b313bc44d1cbf"),
    "__v": 0,
    "ess": {
        "nominal_frequency": 40
    }
}
```

Then we will update that document:
`/usr/local/bin/fims/fims_send -m set -u /dbi/site_controller/assets/ess '{"nominal_voltage": 550.0,"nominal_frequency": 60.0,"start_value": 207,"stop_value": 207}'`
And get this result:
```
{
    "_id": ObjectId("6013368ea05b313bc44d1cbf"),
    "__v": 0,
    "ess": {
        "nominal_voltage": 550,
        "nominal_frequency": 60,
        "start_value": 207,
        "stop_value": 207
    }
}
```
N.B.: We could have used a command like this in the first place to create a more-complex document.

Then we will update that document again:
`/usr/local/bin/fims/fims_send -m set -u /dbi/site_controller/assets/ess/nominal_frequency '45'`
And get this result:
```
{
    "_id": ObjectId("6013368ea05b313bc44d1cbf"),
    "__v": 0,
    "ess": {
        "nominal_voltage": 550,
        "nominal_frequency": 45,
        "start_value": 207,
        "stop_value": 207
    }
}
```

Notes:
* Data is created or replaced at whatever level is indicated by the endpoint.
* If the document you intend to update somehow does not already exist, a new document will be created.

### Get Document
```
Method: `get`
URI: `/dbi/:database/:collection[/:document]`
Body: `{}`
```
FIMS message example: `/usr/local/bin/fims/fims_send -m get -u /dbi/site_controller/assets/ess ''`

### Delete Document
```
Method: `del`
URI: `/dbi/:database/:collection/:document`
Body: ``
```
FIMS message example: `/usr/local/bin/fims/fims_send -m del -u /dbi/site_controller/assets/ess ''`


## Testing

Testing will use a set of FIMS messages to read, write, update, and delete documents in various databases. Both correctly formatted and incorrectly formatted queries will be tested. This module will be considered to be functioning correctly when a set of queries produce the expected results and don't cause errors. The plan is to use Jest for testing. We will create messages in the same form as would arrive via FIMS, process those through the `processEvent(msg)` function that FIMS messages are sent to, and then compare the record inserted into MongoDB with what we expected to be inserted. One test structure in Jest could look something like this:
```
const { MongoClient } = require('mongodb');
const { processEvent } = require('../src/dbi');


// TODO: locate and read test .json file 


describe('insert', () => {
    let databaseConnection;
    let databasesObjectDatabaseName;

    beforeAll(async () => {
        databaseConnection = await MongoClient.connect(`mongodb://localhost:27017/dbi`, { useNewUrlParser: true, useUnifiedTopology: true });
        databasesObjectDatabaseName = await databaseConnection.db(theCollectionName);
    });

    afterAll(async () => {
        await databaseConnection.close();
        await databasesObjectDatabaseName.close();
    });

    for (let i = 0; i < theTestData.length; i++) {
        // TODO: build the following from the test data.
        // This creates `theMsg`, a message identical to what is
        // seen at fims.receiveWithTimeout and then processed in
        // processEvent(theMsg).
        // NOTE: this needs some adjustment
        const theMethod = '';
        const theURI = '';
        const theCollectionName = '';
        const theBody = {};
        const theExpectedRecord = {}; // built from pieces of the above
        const theMsg = {
            method: theMethod,
            uri: theURI,
            replyto: 0,
            body: theBody
        };

        it('object inserted into database should match the FIMS message received', async () => {
            processEvent(theMsg);
            const theInsertedRecord = await databasesObjectDatabaseName.collection(theCollectionName).findOne();
            expect(theInsertedRecord).toMatch(theExpectedRecord);
        });
    };
});
```

There are probably better ways to structure the testing in Jest. I created a simple for loop here just to demonstrate that we would take examples, process them, and then compare them. Refinement of Jest testing processes will happen on acceptance and approval of this testing premise.


## Backward Compatibility

This new functionality will, at the least, largely replace .json configuration files for HybridOS modules. It will require modules to send a FIMS query to retrieve their configuration data instead of reading a .json config file. The configuration data returned to the module using either either the database or the .json config file method will be the same so no other changes would be required of modules to use this functionality.