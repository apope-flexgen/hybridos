import request from 'supertest';
import defaults from 'superagent-defaults';

/**
 *
 * @param server http server from test suite
 * @returns custom supertest instance
 */
const testApiRequest = (server: any) => {
  const req = defaults(request(server));
  req.on('request', function (req: any) {
    const url = new URL(req.url);
    if (!url.pathname.startsWith('/rest')) {
      url.pathname = `/api${url.pathname}`;
    }
    req.url = url.toString();
    return req;
  });
  return req;
};

export default testApiRequest;
