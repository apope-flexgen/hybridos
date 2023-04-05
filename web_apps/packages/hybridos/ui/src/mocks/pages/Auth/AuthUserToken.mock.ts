import { rest } from 'msw';

const authUserTokenMock = rest.get('/api/authenticate-user-token', (req, res, ctx) => {
  // TEMP: If refresh cookie exists and not 'invalidated',
  // return successful response, else return 401.
  if (req.cookies.refreshToken && req.cookies.refreshToken !== 'invalidated') {
    return res(
      ctx.json({
        username: 'fgadmin',
        role: 'admin',
      }),
    );
  }
  return res(ctx.status(401), ctx.json({ message: 'Unauthorized' }));
});

export default authUserTokenMock;
