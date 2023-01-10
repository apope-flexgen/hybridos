# Database Interface ("dbi") documentation


## Approach

At its most basic, `dbi` reads and writes json objects to a NoSQL database (we are currently using MongoDB) in response to queries received via FIMS.


## Note on terminology:

The initial design of `dbi` was changed from a database-per-module architecture to a one-database-for-all architecture, meaning that doing a `show databases` in `mongo` will list `dbi` and not `site_controller`, `web_server`, `ess_controller`, etc. Because this change "shifts" everything up a level, instead of, say, `site_controller` being a database (called `site_controller`) with multiple collections each having their own document, now `site_controller` is a *collection* (within `dbi`) with a single document and that document contains multiple objects. Instead of `:database/:collection/:document` being, for example, `site_controller/assets/ess`, now `:database/:collection/:document` is `dbi/site_controller/assets`. The document in MongoDB is a BSON object that can be treated identically to a JSON object, so now we just have different objects nested within the document. While the architecture has shifted, the concepts have not, so for ease-of-understanding I have left references to "document" below even though they are now technically objects within the document.


## Here is what `dbi` will do with FIMS messages of various methods:
* `set` - creates a new document. This method must be used to create separate documents *if you want to keep separate versions of documents*.
* `put` - updates a document or document part and will create a new document if there's no existing document to be updated. This method can be used all the time if you do not need to keep separate versions of documents.
* `get` - find and return document(s)
* `pub` - NOT USED in this module
* `del` - remove existing document(s)

GET has three special methods: `/_show_collections`, `/_show_documents`, and `/_show_versions` for, respectively, retrieving a list of collections (commonly only for use by the UI), retrieving a list of documents in your collection (e.g., "/site_controller"), or retrieving a list of _versions_ of documents in your collection. An ID obtained from `/_show_versions` can be used to specify a particular document to a PUT, GET, or DEL by adding the suffix `/_id=[the id]` (e.g., `/dbi/testing/assets/_id=6058d51c084fa2d5ab9cef93`) to the endpoint.

The PUT method can be used for document creation and updating at all times if you simply want to keep a single, current document. The SET method must be used to create a new document if you want to create separate versions (separate documents) at a particular endpoint. PUT updates the _**latest**_ document _**unless**_ an `/_id=[id number]` suffix is supplied on the endpoint, in which case it will update that particular document. Similarly, GET retrieves all or part of the _latest_ document unless an ID is supplied. DEL will _delete *all* documents_ at a particular endpoint unless an ID is supplied, in which case it will only delete that particular document.

Whenever a replyTo address is supplied in a request to `dbi`, results are returned as SETs.


## Create or update a document (SET):

`/usr/local/bin/fims/fims_send -m set -u /dbi/site_controller/assets '{"nom_vol": 550.0,"nom_freq": 60.0}'`

creates document in dbi/site_controller:

{ "_id" : ObjectId("6025692b5ccd01f3137b7c2b"), "__v" : 0, "site_controller" : { "assets" : { "nom_vol" : 550, "nom_freq" : 60 } } }
```
{
    "_id" : ObjectId("6021959136892f53d542910c"),
    "__v" : 0,
    "assets" : {
        "nom_vol" : 550,
        "nom_freq" : 60
    }
}
```

**returns response to the querying module:**
```
{
    "assets":{
        "nom_vol":550,
        "nom_freq":60
    }
}
```
---
`/usr/local/bin/fims/fims_send -m set -u /dbi/site_controller/feeders '{"nom_vol": 220.0,"nom_freq": 80.0}'`

creates another document (the second one below) in dbi/site_controller:
```
{
    "_id" : ObjectId("6021959136892f53d542910c"),
    "__v" : 0,
    "assets" : {
        "nom_vol" : 550,
        "nom_freq" : 60
    }
}
{
    "_id" : ObjectId("602195d336892f53d5429116"),
    "__v" : 0,
    "feeders" : {
        "nom_vol" : 220,
        "nom_freq" : 80
    }
}
```

**returns response to the querying module:**
```
{
    "feeders":{
        "nom_vol":220,
        "nom_freq":80
    }
}
```
---
**NOTE: this is a NEGATIVE example. Don't do this if you're trying to update *an element within* a document!**

`/usr/local/bin/fims/fims_send -m set -u /dbi/site_controller/feeders '{"nom_freq": 111.0}'`

*replaces the body* of `feeders` document in dbi/site_controller:
```
{
    "_id" : ObjectId("6021959136892f53d542910c"),
    "__v" : 0,
    "assets" : {
        "nom_vol" : 550,
        "nom_freq" : 60
    }
}
{
    "_id" : ObjectId("602195d336892f53d5429116"),
    "__v" : 0,
    "feeders" : {
        "nom_freq" : 111
    }
}
```
---
**NOTE: Here is the correct way to update *an element within* a document:**

`/usr/local/bin/fims/fims_send -m set -u /dbi/site_controller/feeders/nom_freq '111.0'`

updates *an element within* the `feeders` document in dbi/site_controller:
```
{
    "_id" : ObjectId("6021959136892f53d542910c"),
    "__v" : 0,
    "assets" : {
        "nom_vol" : 550,
        "nom_freq" : 60
    }
}
{
    "_id" : ObjectId("602195d336892f53d5429116"),
    "__v" : 0,
    "feeders" : {
        "nom_vol" : 220,
        "nom_freq" : 111
    }
}
```
---
`/usr/local/bin/fims/fims_send -m set -u /dbi/site_controller/feeders '{"nom_vol": 990.0,"nom_freq": 99.0}'`

updates *the entire* `feeders` document in dbi/site_controller:
```
{
    "_id" : ObjectId("6021959136892f53d542910c"),
    "__v" : 0,
    "assets" : {
        "nom_vol" : 550,
        "nom_freq" : 60
    }
}
{
    "_id" : ObjectId("602195d336892f53d5429116"),
    "__v" : 0,
    "feeders" : {
        "nom_vol" : 990,
        "nom_freq" : 99
    }
}
```

**returns response to the querying module:**
```
{
    "feeders":{
        "nom_vol":990,
        "nom_freq":99
    }
}
```
---

## Get a document (GET):

`/usr/local/bin/fims/fims_send -m get -u /dbi/site_controller/assets ''`

**returns response to the querying module:**
```
{
    "nom_vol":550,
    "nom_freq":60
}
```
---
`/usr/local/bin/fims/fims_send -m get -u /dbi/site_controller ''`

**returns response to the querying module:**
```
{
    "assets":{
        "nom_vol":550,
        "nom_freq":60
    },
    "feeders":{
        "nom_vol":990,
        "nom_freq":99
    }
}
```
---
`/usr/local/bin/fims/fims_send -m get -u /dbi/_show_collections ''`

**returns response to the querying module:**
```
{
    "collections":["site_controller", "ess_controller", "web_server"]
}
```
---
`/usr/local/bin/fims/fims_send -m get -u /dbi/site_controller/_show_documents ''`

**returns response to the querying module:**
```
{
    "documents":["assets", "feeders]
}
```
---
`/usr/local/bin/fims/fims_send -m get -u /dbi/site_controller/assets/_show_versions ''`

**returns response to the querying module:**
An id can be used to specify a particular document by adding `/_id=[id number]` to the end of a PUT, GET, or DEL query, e.g., `/usr/local/bin/fims/fims_send -m put -u /dbi/site_controller/assets/_id=6058d51c084fa2d5ab9cef93 -r /me '{"nominal_voltage":545,"nominal_frequency":99,"start_value":979,"stop_value":878}'`
```
{
    "versions":[
        {"id":"6058ded2f281b24a2cf4fa5d","created":"2021-03-22T18:15:46.772Z","updated":"2021-03-22T18:15:46.772Z"},
        {"id":"6058ded2f281b24a2cf4fa5c","created":"2021-03-22T18:15:46.134Z","updated":"2021-03-22T18:15:46.134Z"},
        {"id":"6058ded0f281b24a2cf4fa5b","created":"2021-03-22T18:15:44.215Z","updated":"2021-03-22T18:15:44.215Z"},
        {"id":"6058d51c084fa2d5ab9cef93","created":"2021-03-22T17:34:20.018Z","updated":"2021-03-22T17:34:46.840Z"}
    ]
}
```
---

## Delete a document (DEL):

`/usr/local/bin/fims/fims_send -m del -u /dbi/site_controller/assets ''`

deletes the `assets` document leaving `feeders` alone:
```
{
    "_id" : ObjectId("602195d336892f53d5429116"),
    "__v" : 0,
    "feeders" : {
        "nom_vol" : 990,
        "nom_freq" : 99
    }
}
```

**returns response to the querying module:**
```
{"deletedCount":1}
```
---
`/usr/local/bin/fims/fims_send -m del -u /dbi/site_controller ''`

deletes the entire `site_controller` object. This could have been done first, deleting all (2) documents in `site_controller`.

**returns response to the querying module:**
```
{"deletedCount":1}
```
---

## See [Test Data](../testing/testData.json) in /testing for more examples of queries and results