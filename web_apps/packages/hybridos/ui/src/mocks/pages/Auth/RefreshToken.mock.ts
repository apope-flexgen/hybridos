import { rest } from 'msw';

const refreshTokenMock = rest.get('/api/refresh_token', (req, res, ctx) => {
  if (req.cookies.refreshToken && req.cookies.refreshToken !== 'invalidated') {
    return res(
      ctx.json({
        username: 'fgadmin',
        role: 'admin',
        accessToken: 'refreshed-access-token',
      }),
    );
  }
  return res(ctx.status(401), ctx.json({ message: 'Unauthorized' }));
});

export default refreshTokenMock;
