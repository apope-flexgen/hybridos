import { Inject, Injectable } from '@nestjs/common';
import { PassportStrategy } from '@nestjs/passport';
import { Strategy, ExtractJwt } from 'passport-jwt';
import { APP_ENV_SERVICE, IAppEnvService } from 'src/environment/appEnv.interface';

@Injectable()
export class AccessTokenMfaStrategy extends PassportStrategy(Strategy, 'accessToken-mfa') {
  constructor(
    @Inject(APP_ENV_SERVICE)
    private appEnvService: IAppEnvService,
  ) {
    super({
      jwtFromRequest: ExtractJwt.fromAuthHeaderAsBearerToken(),
      ignoreExpiration: false,
      secretOrKey: appEnvService.getJwtSecretKeyMFA(),
    });
  }

  static key = 'accessToken-mfa';

  async validate(payload: any) {
    return payload.oneTime === 'mfa' ? { userId: payload.sub, username: payload.username } : null;
  }
}
