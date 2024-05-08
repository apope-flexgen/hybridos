import { createParamDecorator, ExecutionContext } from '@nestjs/common';
import { JwtService } from '@nestjs/jwt';
import { User } from 'src/users/dtos/user.dto';

export const UserFromAccessToken = createParamDecorator(
  (_data: string, ctx: ExecutionContext): User => {
    const request = ctx.switchToHttp().getRequest();
    const jwtService = new JwtService();
    if (request.headers['authorization']) {
      const token = request.headers['authorization'].split(' ')[1];
      const decoded = jwtService.decode(token) as any;
      return {
        username: decoded?.sub,
        role: decoded?.role,
      };
    }
    return null;
  },
);
