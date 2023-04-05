import { rest } from 'msw';

export const passExpMock = rest.post('/api/login/passExp', (req, res, ctx) => {
  const REFRESH_TOKEN = 'passExp-refresh-token';
  const ACCESS_TOKEN = 'passExp-access-token';
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

export const passExpMockOldPassword = rest.post('/api/login/passExp', (req, res, ctx) => res(
  ctx.status(401),
  ctx.json({
    message: 'New password must not match current password',
  }),
));
