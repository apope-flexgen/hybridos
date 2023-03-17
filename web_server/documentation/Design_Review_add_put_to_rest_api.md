# Add PUT functionality to REST API

## Goals

Our customers occasionally need to send override commands through the REST API, doing so requires the ability to PUT to the API. When complete, the customer will be able to securely PUT commands to the API when they use the appropriate credentials.


## Affected Repos

web_server is the only repo that will be affected.


## Approach

The REST API was initially created to allow GETs of HybridOS statuses (such as site summaries and metrics) for secure external monitoring of a site or multiple sites. API GET functionality already exists in HybridOS. In respect to validating credentials and confirming access to particular URIs, PUT functionality uses the same frameworks as the existing GET functionality, so no new functions or methods will need to be created for security. New methods for individuating permissions and URI paths will be discussed in separate Pull Requests/Design Reviews.


## Interface

The interfaces for this functionality - the REST API and FIMS - will be used in a manner identical to the existing API GET functionality. Within web_server, an API .put function will handle PUTs while the existing .get function handles GETs. Other than these slightly different "handler" functions, internal authentication, validation, and routing is the same for either functionality.


## Testing

PUT functionality can and will be tested with Postman and other methods of sending an API request. We will test authentication, URIs, and values - with both valid and invalid data - to confirm that the functionality works as it should, and fails gracefully and/or properly rejects erroneous or dubious requests when it should. A valid request is one which carries the appropriate credentials, points to a valid URI for which the credentials have permission, and supplies a valid value to send to that URI. An invalid request is one that does not meet all three of those valid criteria. Invalid requests may be answered with an error code or no reply at all, depending on the nature of the problem. Error codes are supplied for cases that appear to be legitimate requests with small errors. For security reasons, we will not reply to requests that appear to be malicious.

## Backward Compatibility

PUT functionality has no impact on backward compatibility. It does not modify an existing functionality, rather it adds new functionality.


## Configuration

In the same manner as the existing GET access on the API, the user - our customer - would send an HTTP request to the IP address of the HybridOS instance (specifically where web_server exists). That HTTP request would include the IP address, the URI to access, and the value to be PUT. Credentials would be sent in the HTTP authentication header using the "Basic Auth" method.

In the following example, "https://10.1.1.1/rest" is the essential part of the IP address, "/features/active_power" is the URI, and "fr_baseload_kW_cmd/0" is the control point and value ("0") that is being PUT. "HTTP" requests must be sent using HTTPS.

`https://10.1.1.1/rest/features/active_power/fr_baseload_kW_cmd/0`

Authentication uses the "Basic Auth" method. An HTTP Authorization header containing a base64-encoded username:password string is passed in the request header.