# dbi
General database interface from FIMS to MongoDB or other backends
### Knowledge about dbi:
* In the construction of a URI the first fragment will always be a database, the second will be a collection, the third will be a document.
* Documents are entries into a collection, just like collections are entries in a database
* For example: /dbi/site_controller/new, will make/go into the database dbi then make/go into the collection site_controller and then will add a document new into collection site_controller.

* There are special URIs that will allow you to see collections of a database, documents of a collection, or a map of documents in a collection

`fims_send -m get -u /dbi/show_collections`, will give you a list of all the current collections in the database
* Example output: `["site_controller"]`

`fims_send -m get -u /dbi/site_controller/show_documents`, will give you a list of all the documents associated with the collection
* Example output: `["modes", "config"]`

`fims_send -m get -u /dbi/site_controller/show_map`, will give you a map of what each document looks like in that collection
* Example output:
```json
{
  "config": {
    "_doc": "config",
    "_id": "61ae1bf6fb322c6e81ed9bc7",
    "_version": "2021-12-06T09:19:34.368-05:00",
    "concert": 100,
    "need": "water"
  },
  "modes": {
    "_doc": "modes",
    "_id": "61ae2305d81ffeb290b9bc2a",
    "_version": "2021-12-06T09:49:41.594-05:00",
    "help": "me",
    "why": 10
  }
}
```

### Identifiers that are in each Document of a collection:
* "_id": This allows the program know what document it is updating
* "_doc": This allows the program to know what document it is assigned to when retrieving from mongo, to make sure that local map and mongo are the same [DEPREACTED -- just use _id]
* "_version": This is a time stamp for when it was created

### These examples are assuming you are starting with an empty database

### Sets
* Using a set in fims can overwrite or add to a specific document.
* Adding a value to the database: 
`fims_send -m set -u /dbi/site_controller/configs/data '{"value": 100}'`

* This command will create the database, collection and document with data having a value 100
```json
{
	"_id": "(a unique identifier made by mongo)",
	"_doc": "config",
	"_version": "(time stamp)",
	"data": 100,
}
```
* Adding a map to the database:

	* After using the command above if this command was executed: `fims_send -m set -u /dbi/site_controller/configs/new '{"temp": 32, "pressure": 20}'`

* This command will had a new part to the document where it will look like this
```json
{
	"_id": "(a unique identifier made by mongo)",
	"_doc": "config",
	"_version": "(time stamp)",
	"data": 100,
	"new":{
		"temp": 32,
		"pressure": 20
	}
}
```
* As long as what is being set is a new name of something it will always be added.
* Depending on the URI a part of the document can be overwritten.
* If this command was to be ran after the other two, it would replace everything that you added before.
`fims_send -m set -u /dbi/site_controller/configs '{"temp": 32, "pressure": 20}'`
* The result will look like this:
```json
{
	"_id": "(a unique identifier made by mongo)",
	"_doc": "configs",
	"_version": "(time stamp)",
	"temp": 32,
	"pressure": 20
}
```
* Be careful when you are setting data


* Two fragment Sets
	* These sets are strictly used for copying collections to an empty database on another machine.
	* Correct process to use the two frag sets
		* Do a get on the collection: `fims_send -m get -u /dbi/site_controller -r /$$ >> <some file>` or `fims_send -m get -u /dbi/site_controller/show_map -r /$$ >> <some file>`
		* Copy that file to another machine
		* Do a set on that collection (database must be empty):  `fims_send -m get -u /dbi/site_controller -f <some file>`

### Gets
* Gets allow you to retrieve data that is within a Document depending on the URI that is given
* From the example this command will get the entire document
* This command will get this data from the example:
`fims_send -m get -u /dbi/site_controller/configs`
```json
{
	"_id": "(a unique identifier made by mongo)",
	"_doc": "configs",
	"_version": "(time stamp)",
	"temp": 32,
	"pressure": 20,
}
```
* However if this command was sent
`fims_send -m get -u /dbi/site_controller/configs/temp`
* it would return 32

* If we look at the information before the main part of document was overwritten and we ran this command
`fims_send -m get -u /dbi/site_controller/configs/new`
* We would get:
```json
{
	"temp": 32,
	"pressure": 20
}
```
* DBI currently onlt supports "gets" that are 3 fragments or longer

### Delete

* Delete works similar to get but it will delete the key of the last fragment of the URI

* If the database has this document:
```json
{
	"_id": "(a unique identifier made by mongo)",
	"_doc": "config",
	"_version": "(time stamp)",
	"data": 100,
	"new":{
		"temp": 32,
		"pressure": 20
	}
}
```
* If this command were to be executed:
`fims_send -m del -u /dbi/site_controller/configs/data`

* Then the database would look like this
```json
{
	"_id": "(a unique identifier made by mongo)",
	"_doc": "config",
	"_version": "(time stamp)",
	"new":{
		"temp": 32,
		"pressure": 20
	}
}
```
* Now you can delete elements of a map with this command: 
`fims_send -m del -u /dbi/site_controller/configs/new/temp`
* Will result in this:
```json
{
	"_id": "(a unique identifier made by mongo)",
	"_doc": "config",
	"_version": "(time stamp)",
	"new":{
		"pressure": 20
	}
}
```
* If we one step farther by deleting pressure we will get:
`fims_send -m del -u /dbi/site_controller/configs/new/pressure`
```json
{
	"_id": "(a unique identifier made by mongo)",
	"_doc": "config",
	"_version": "(time stamp)",
	"new":{}
}
```
* I believe you can delete the identifiers, but i hope no one does because it is not a real key that is used in a map, since they have "_"