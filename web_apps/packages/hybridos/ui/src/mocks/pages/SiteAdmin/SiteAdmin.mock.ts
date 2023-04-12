import { rest } from 'msw';

const mocksiteAdmins = {
  password: {
    multi_factor_authentication: false,
    password_expiration: false,
    minimum_password_length: 8,
    maximum_password_length: 128,
    password_expiration_interval: '8d',
    old_passwords: 0,
    password_regular_expression: JSON.stringify(
      // TODO: fix lint
      // eslint-disable-next-line no-useless-escape
      /^(?=.*[a-z])(?=.*[A-Z])(?=.*[0-9])(?=.*[!"#\$%&'\*\+,\.\/:;=\\?@\^`\|~])/,
    ),
  },
  radius: {
    is_enabled: false,
    ip_address: '127.0.0.1',
    port: '1812',
    secret_phrase: 'testing123',
    wait_time: 5000,
    is_local_auth_disabled: false,
  },
};

export const radiusTestMock = rest.get('/radius-test', (req, res, ctx) => res(ctx.json({ message: 'Mock test response' })));

export const siteAdminMock = rest.get('/site-admins', (req, res, ctx) => res(ctx.json(mocksiteAdmins)));

export const siteAdminPostMock = rest.post('/site-admins', async (req, res, ctx) => {
  const newsiteAdmins = await req.json();
  return res(ctx.json(newsiteAdmins));
});
