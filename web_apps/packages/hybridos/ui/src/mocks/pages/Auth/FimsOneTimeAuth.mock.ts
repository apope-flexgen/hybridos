import { rest } from 'msw';

const fimsOneTimeAuthMock = rest.get('/api/fims/one-time-auth', (req, res, ctx) => {
  if (req.headers.get('Authorization')) {
    return res(
      ctx.json({
        username: 'fgadmin',
        role: 'admin',
        accessToken: 'fims-one-time-auth-access-token',
      }),
    );
  }
  return res(ctx.status(401), ctx.json({ message: 'Unauthorized' }));
});

export default fimsOneTimeAuthMock;
