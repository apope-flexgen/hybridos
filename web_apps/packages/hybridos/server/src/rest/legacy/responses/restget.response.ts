export const LEGACY_REST_GET_RESPONSE_DESCRIPTION = `If the requested URI is an aggregated endpoint, this JSON object will contain the data for each child URI.
If the requested URI is not an aggregated endpoint and the requested data is clothed, then the JSON response will contain the entire body of the clothed data.
If the requested URI is not an aggregated endpoint and the requested data is naked, then the JSON response will be {uri: <requested_uri>, value: <value>}`;

export class LegacyRestGetResponse {}
