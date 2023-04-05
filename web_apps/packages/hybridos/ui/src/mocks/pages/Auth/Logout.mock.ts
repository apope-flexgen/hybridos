import { rest } from 'msw';

const logoutMock = rest.post('/api/logout', (req, res, ctx) => res(
  ctx.status(200),
  ctx.cookie('refreshToken', 'invalidated'),
  ctx.json({
    message: 'Logged out.',
  }),
));

export default logoutMock;
