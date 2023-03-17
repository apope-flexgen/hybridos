# How to test super endpoints

After building, do `npm run test -t endpoints` to run the super endpoints unit tests.

In order to test the API speed, an instance of a controller must be running that pubs data. The config requires REST permissions and a set of endpoints in web_ui. An example can be found in the `test/endpointsConfig` branch of `config`. Once a controller is running, postman can be used to send requests.