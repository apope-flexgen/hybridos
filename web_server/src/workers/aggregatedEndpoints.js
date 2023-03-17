/* eslint-disable no-param-reassign */
/**
 * Initializes endpoints object based on configuration
 * @param {object} config contains list of URI endpoints to initialize
 * @returns {object} URI keys with empty values as initial data structure
 */
function initializeAggregatedEndpoints(config) {
  const initializedAggregatedEndpoints = {};
  if (config.aggregatedEndpoints) {
    Object.keys(config.aggregatedEndpoints).forEach((topLevelEndpoint) => {
      initializedAggregatedEndpoints[topLevelEndpoint] = {};
      config.aggregatedEndpoints[topLevelEndpoint].forEach((endpoint) => {
        initializedAggregatedEndpoints[topLevelEndpoint][endpoint] = {};
      });
    });
  }

  return initializedAggregatedEndpoints;
}

/**
 * Does a fims get on each endpoint in the endpoints object
 * @param {object} aggregatedEndpoints endpoints object
 * @param {function} fimsSend helper function that sends msg, or tests msg
 */
function getAggregatedEndpoints(aggregatedEndpoints, fimsSend) {
  Object.keys(aggregatedEndpoints).forEach((topLevelEndpoint) => {
    Object.keys(aggregatedEndpoints[topLevelEndpoint]).forEach((endpoint) => {
      fimsSend({
        method: "get",
        uri: `${topLevelEndpoint}/${endpoint}`,
        replyto: `/aggregate${topLevelEndpoint}/${endpoint}`,
        body: null,
      });
    });
  });
}

/**
 * Merges data into endpoints object at specefied URI key
 * @param {object} aggregatedEndpoints endpoints object
 * @param {string} uri URI to merge on
 * @param {object} newData data to merge
 */
function mergeEndpoints(aggregatedEndpoints, uri, newData) {
  const uriLastIndex = uri.lastIndexOf("/");
  const topLevelEndpoint = uri.slice(0, uriLastIndex);
  const endpoint = uri.slice(uriLastIndex + 1);

  // Check if property exists for PUBS only, gets should already be defined
  if (!aggregatedEndpoints[topLevelEndpoint][endpoint]) {
    aggregatedEndpoints[topLevelEndpoint][endpoint] = {};
  }

  Object.assign(aggregatedEndpoints[topLevelEndpoint][endpoint], newData);
}

/**
 * Filters out endpoints that user does not have permission to view
 * @param {object} aggregatedEndpoints endpoints object
 * @param {string} path parent path for endpoint
 * @param {string} userName username to check access permissions
 * @param {string} role role to check access permissions
 * @param {function} hasPermissions helper function that checks permissions
 * @returns {object} endpoints object with unauthorized keys removed
 */
function filterAndReduceRequestedObjects(
  aggregatedEndpoints,
  path,
  userName,
  role,
  hasPermissions
) {
  const authorizedTopLevelEndpoint = Object.keys(aggregatedEndpoints[path])
    .filter((endpoint) => {
      const fullUri = `${path}/${endpoint}`;

      return (
        hasPermissions("read", {
          userName,
          role,
          accessPoint: fullUri,
        }) ||
        hasPermissions("readWrite", {
          userName,
          role,
          accessPoint: fullUri,
        })
      );
    })
    .reduce(
      (obj, endpoint) => ({
        ...obj,
        [endpoint]: aggregatedEndpoints[path][endpoint],
      }),
      {}
    );

  return authorizedTopLevelEndpoint;
}

module.exports = {
  initializeAggregatedEndpoints,
  getAggregatedEndpoints,
  mergeEndpoints,
  filterAndReduceRequestedObjects,
};
