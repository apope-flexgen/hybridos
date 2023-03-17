
# Aggregated URIs

## Goals

The goal is to allow for a configurable object for web_server that will aggregate selected children from the parent URI and allow for the object to be retrievable through the REST api.

## Affected Repos

web_server, config

## Approach

The general idea is to create an object that holds all the data for every topLevel endpoint, then populate that object with PUBs and GETs. The reason we need GETs is that certain endpoints do not publish their entire object on PUBs on DNP3 (i.e. /sites). On every GET and PUB, the data will be merged into the current object and update its values. When a user uses the REST api to get a topLevel endpoint, the response will first verify the user's permissions then send the object of interest.

A new variable will be declared in web_server.js. It will pull from the web_ui config. In the config a new object will be declared listing all of the topLevel endpoints that should be aggregated with the keys representing the topLevel endpoints to be aggregated. (e.g. `"/sites":["alvin","brazoria"]`

```
let aggregatedEndpoints = {};
Object.keys(siteConfiguration.aggregatedEndpoints).forEach((topLevelURI) => {
	aggregatedEndpoints[topLevelURI] = {};
	siteConfiguration.aggregatedEndpoints[topLevelURI].forEach((uri) => {
		aggregatedEndpoints[topLevelURI][uri] = {};
	}
};
```

The object will be updated both through PUBs and through GETs. This is necessary because some of the desired paths do NOT have all values pubbed. The gets will be sent with the heartbeat, and still received through the `listenForFIMS()` function.
```
function getAggregatedEndpoints() {
	for (const [key, value] of Object.entries(aggregatedEndpoints)) {
		Object.keys(value).forEach((uri) => {
			const  msg  = {
			method: 'get',
			uri: `${key}/${uri}`,
			replyto: `/aggregate${key}/${uri}`,
			body: null,
		};
		fimsApi.send(msg);
		};
	}
}

beat((count) => {
	countMessages(count);
	sendHeartbeatMsg(count);
	getAggregatedEndpoints();
}, 1000);
```

In the FIMS loop, a new check will be added to see if the URI of the data exists in the topLevel endpoints object. Object.assign will copy all  properties from one or more source objects to a target object. It returns the target object. https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object/assign
```
listenForFIMS(data) => {
	if (data) {
		if (pathFirstPart === 'rest') {
			...
		} else if (pathFirstPart === 'aggregate') {
			const requestURI = data.uri.replace('/aggregate', '');
			const parentEndpoint = requestURI.slice(0, data.uri.lastIndexOf('/');
			const child = requestURI.slice(data.uri.lastIndexOf('/') + 1);
			Object.assign(aggregatedEndpoints[parentEndpoint][child], data.body);
		} else {
			if (data.method === 'pub') {
				if (aggregatedEndpoints.hasOwnProperty(data.uri.slice(0, data.uri.lastIndexOf('/'))) {
					const parentEndpoint = data.uri.slice(0, data.uri.lastIndexOf('/');
					const child = data.uri.slice(data.uri.lastIndexOf('/') + 1);
					Object.assign(aggregatedEndpoints[parentEndpoint][child], data.body);
				}
			}
		}
		...regular FIMS processing
	}
}
```
For the rest API the path will be checked to see if it exists in the topLevel endpoint. If it does, then it will check each child URI endpoint to see if the user/role has permission to view it. If the user/role does not, it will delete it from the object. Then it will send the cleaved object. If the path is not a topLevel endpoint, then the REST api will function as before.
```
router.get(path) {
	if (aggregatedEndpoints.hasOwnProperty(path) {
		const aggregateObject = Object.keys(aggregatedEndpoints[path])
			.filter((uri) => (accessPerRole[userName] && (accessPerRole[userName].indexOf(`${path}/${uri} read`) > -1 || accessPerRole[userName].indexOf(`${path}/${uri} readWrite`) > -1)) || (accessPerRole[role].indexOf(`${path}/${uri} read`) > -1 || accessPerRole[role].indexOf(`${path}/${uri} readWrite`) > -1))
			.reduce((obj, key) => {
				return {
					...obj,
					[key]: aggregatedEndpoints[path][key]
				};
			}, {});
		queryID.res.status(200).send(aggregatedEndpointsCopy);
	} else {
		...original FIMS 'get' functionality...
	}
}
```

## Interface
The interface adds functionality to the REST api. If the URI is in the aggregated object then the REST path will return the aggregated object body while verifying user/role permissions for each child and also the parent object. Otherwise the REST api will do a get through FIMS, which is the original behavior. The URIs are defined in the web_ui.json file in the config repository. 

## Testing

Testing will be done with jest and supertest. Jest will be used for unit tests, integration tests, and end to end tests. supertest will be added to allow for jest to make requests to an express server. 

Unit tests will include object merging and object creation. The main concern is that deep properties should be merged/updated correctly. Object.assign does not do a deep merge so if this is needed, lodash.merge could be an option. Tests will also test endpoint object creations and uri parsing.

Integration test will combine functions from unit testing to do endpoint object creation from a provided config object, endpoint object merging from parsed uri, and sending a filtered object from a parsed URI.

End to end testing will use supertest to send requests to the web_server. It will test from the use case of the potential user. Mostly it will check from configuration to request.
Jest will be used to set up and start certain processes, mainly web_server and fims, using node child_processes. Using nodeFIMS we can mock pub, set, and gets to simulate data updates and retrieval.

Testing will mainly be just running tests, but custom tests can be added if needed.

## Backward Compatibility

This addition will be fully backwards compatible. If there is no change to the config in web_server.json, then this code won't change any functionality. 

## Configuration

The configuration is a list of URIs

web_ui.json or web_server.json
```
{
	"aggregatedEndpoints": {
		"/sites": ["alvin", "brazoria"],
		"/components": ["alvin", "brazoria"],
		"/assets": ["bms", "pcs"],
	}
}
```