import { rest } from 'msw';

export const loginMock = rest.post('/api/login', (req, res, ctx) => res(
  ctx.cookie('refreshToken', 'fake-flexgen-token'),
  ctx.status(200),
  ctx.json({
    username: 'fgadmin',
    role: 'admin',
    accessToken: 'fake-access-token',
  }),
));

export const loginMockUnauthorized = rest.post('/api/login', (req: any, res: any, ctx: any) => res(
  ctx.status(401),
  ctx.json({
    message: 'Unauthorized',
  }),
));

export const loginPassExpMock = rest.post('/api/login', (req, res, ctx) => {
  const ACCESS_TOKEN = 'onetime-passexp-access-token';
  return res(
    ctx.status(200),
    ctx.json({
      username: 'fgadmin',
      passwordExpired: true,
      accessToken: ACCESS_TOKEN,
    }),
  );
});

export const loginMfaMock = rest.post('/api/login', (req, res, ctx) => {
  const ACCESS_TOKEN = 'onetime-mfa-access-token';
  return res(
    ctx.status(200),
    ctx.json({
      username: 'fgadmin',
      mfaRequired: true,
      qrCode: 'QR_CODE_STRING',
      accessToken: ACCESS_TOKEN,
    }),
  );
});
