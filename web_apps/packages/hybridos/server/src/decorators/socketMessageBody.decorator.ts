import { createParamDecorator, ExecutionContext } from '@nestjs/common';

export const SocketMessageBody = createParamDecorator((_: unknown, ctx: ExecutionContext) => {
  const request = ctx.switchToWs().getData();
  return request.data;
});
