import { Inject, Injectable } from '@nestjs/common';
import { PassportStrategy } from '@nestjs/passport';
import { Strategy } from 'passport-jwt';
import { RefreshTokenService } from '../refreshTokenService';
import { APP_ENV_SERVICE, IAppEnvService } from 'src/environment/appEnv.interface';

@Injectable()
export class RefreshTokenStrategy extends PassportStrategy(Strategy, 'refresh-token') {
  constructor(
    private readonly refreshTokenService: RefreshTokenService,
    @Inject(APP_ENV_SERVICE)
    private appEnvService: IAppEnvService,
  ) {
    super({
      jwtFromRequest: refreshTokenService.extractRefreshTokenFromRequest,
      ignoreExpiration: false,
      secretOrKey: appEnvService.getJwtSecretKey(),
      passReqToCallback: true,
    });
  }

  async validate(req, payload: any) {
    const refreshToken = this.refreshTokenService.extractRefreshTokenFromRequest(req);
    return this.refreshTokenService.containsRefreshToken(refreshToken)
      ? { ...payload, refreshToken }
      : null;
  }
}
