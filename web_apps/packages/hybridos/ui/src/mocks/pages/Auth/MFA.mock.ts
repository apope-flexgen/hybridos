import { rest } from 'msw';

export const mfaMock = rest.post('/api/login/mfa', (req, res, ctx) => {
  const REFRESH_TOKEN = 'mfa-refresh-token';
  const ACCESS_TOKEN = 'mfa-access-token';
  return res(
    ctx.cookie('refreshToken', REFRESH_TOKEN),
    ctx.status(200),
    ctx.json({
      username: 'fgadmin',
      role: 'admin',
      accessToken: ACCESS_TOKEN,
    }),
  );
});

export const mfaMockInvalidCode = rest.post('/api/login/mfa', (req, res, ctx) => res(
  ctx.status(401),
  ctx.json({
    message: 'Incorrect TOTP Code',
  }),
));
