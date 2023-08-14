import { Injectable, UnauthorizedException } from '@nestjs/common';
import { AuthGuard } from '@nestjs/passport';

@Injectable()
export class RefreshTokenGuard extends AuthGuard('refresh-token') {
  handleRequest(err: any, user: any, info: any) {
    if (err || !user) {
      const message = info ? info.message : 'Unauthorized';
      throw err || new UnauthorizedException({ message: message, statusCode: 401 });
    }
    return user;
  }
}
