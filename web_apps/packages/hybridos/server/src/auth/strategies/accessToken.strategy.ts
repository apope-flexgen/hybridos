import { Inject, Injectable } from '@nestjs/common';
import { PassportStrategy } from '@nestjs/passport';
import { Strategy, ExtractJwt } from 'passport-jwt';
import {
  IValidAccessTokenService,
  VALID_ACCESS_TOKEN_SERVICE,
} from 'src/auth/interfaces/validAccessToken.service.interface';
import { APP_ENV_SERVICE, IAppEnvService } from 'src/environment/appEnv.interface';

@Injectable()
export class AccessTokenStrategy extends PassportStrategy(Strategy) {
  constructor(
    @Inject(VALID_ACCESS_TOKEN_SERVICE)
    private readonly validAccessTokenService: IValidAccessTokenService,
    @Inject(APP_ENV_SERVICE)
    private appEnvService: IAppEnvService,
  ) {
    super({
      jwtFromRequest: ExtractJwt.fromAuthHeaderAsBearerToken(),
      ignoreExpiration: false,
      secretOrKey: appEnvService.getJwtSecretKey(),
      passReqToCallback: true,
    });
  }

  async validate(req, payload: any) {
    const accessToken = this.validAccessTokenService.extractAccessTokenFromRequest(req);
    return (await this.validAccessTokenService.validateAccessToken(accessToken))
      ? { role: payload.role, username: payload.sub }
      : null;
  }
}
