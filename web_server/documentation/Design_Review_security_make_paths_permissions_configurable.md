# Security: Make paths and permissions configurable

## Goals

To allow more granularity in permissions so user "A" can access a superset or a subset of the paths that user "B" can access.


## Affected Repos

The web_server repo is the only repo that will be materially affected. A new `web_server` directory with a `permissions.json` file will be put in the `/opt` directory on the HybridOS installation to load new permissions. That `permissions.json` file will be deleted after its information is read into, encrypted, and stored in the authentication database (`hybridos_authentication`). The `hybridos_authentication` mongo database will be affected, but this database is only used by `web_server` so, again, other repos will not be affected.

**NOTE: we will need to add some permissions setup to the spec file to allow for writing to the `/opt` directory.**


## Approach

To date, the security requirement for HybridOS has been rudimentary: user accounts are created in the UI with the role of "user", "rest", "admin", or "developer" and, on successful authentication, the user is granted access to the resources (the "paths" or URIs) *available to that role*. All "user" accounts have access to the same set of paths, all "rest" accounts have access to the same set of paths, etc.

There is no present need to change this arrangement for users of the HybridOS UI, but there may be in future. There is, however, a present need for the REST API to base access on *individual accounts* versus roles. Granting both PUT and GET access to everyone in the "rest" role presents security risks. Simply creating "rest-put" and "rest-get" roles is likewise a security risk because some PUT actions are far more sensitive than others and some GET actions may retrieve more sensitive or valuable information than others. There needs to be more granularity in permissions so we can allow user "A" to access a superset or a subset of the paths that user "B" can access.

To allow for both granularity in permissions and configurability of those permissions, we will use a .json configuration file to load permissions information for both roles and (optionally) individual users into the `hybridos_authentication` database. If an individual username appears in the permissions object, that user will get those permissions. If a username does not appear in the permissions object, that user will get the "generic" permissions based on their role.


## Security Considerations

Because a .json file listing permissions would be a very valuable find for anyone with malicious intent, we will process any existing, valid `permissions.json` file at the startup of web_server and then overwrite or delete `permissions.json` so as to remove the information. The .json information will be encrypted and stored in the `hybridos_authentication` database for retrieval by web_server. Information in a new, valid `permissions.json` will overwrite any existing permissions information in `hybridos_authentication`. Some minimal default access to the UI and the REST API based on a user's role will be hard-coded in web_server in case an erroneous or corrupt `permissions.json` file disrupts proper access.


## Interface

The interface to this functionality is the `permissions.json` file. Whatever is put in that file will overwrite any existing permissions information in `hybridos_authentication`. Some minimal default access to the UI and the REST API based on a user's role will be hard-coded in web_server in case an erroneous or corrupt `permissions.json` file disrupts proper access.


## Testing

Permissions can and will be tested with HybridOS's UI along with Postman and other methods of sending an API request. We will test authentication and URIs to confirm that the functionality works as it should, and fails gracefully and/or properly rejects erroneous or dubious requests when it should. A valid request is one which carries the appropriate credentials and points to a valid URI for which the credentials have permission. An invalid request is one that does not meet the valid criteria. Invalid requests may be answered with an error code or no reply at all, depending on the nature of the problem. Error codes are supplied for cases that appear to be legitimate requests with small errors. For security reasons, we will not reply to requests that appear to be malicious.


## Backward Compatibility

Introducing more granularity in permissions has no impact on backward compatibility. It does not modify an existing functionality, rather it adds new functionality.


## Configuration

A new `web_server` directory with a `permissions.json` file will be put in the `/opt` directory on the HybridOS installation to load new permissions. That `permissions.json` file will be deleted after its information is read into, encrypted, and stored in the authentication database (`hybridos_authentication`). The `permissions.json` file will contain a .json object spelling out access for each role or username. See the example at [example_permissions.json](example_permissions.json).