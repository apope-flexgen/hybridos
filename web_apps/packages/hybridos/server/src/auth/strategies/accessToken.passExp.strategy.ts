import { Inject, Injectable } from '@nestjs/common';
import { PassportStrategy } from '@nestjs/passport';
import { Strategy, ExtractJwt } from 'passport-jwt';
import { APP_ENV_SERVICE, IAppEnvService } from 'src/environment/appEnv.interface';

@Injectable()
export class AccessTokenPassExpStrategy extends PassportStrategy(
  Strategy,
  'accessToken-password-expiration',
) {
  constructor(
    @Inject(APP_ENV_SERVICE)
    private appEnvService: IAppEnvService,
  ) {
    super({
      jwtFromRequest: ExtractJwt.fromAuthHeaderAsBearerToken(),
      ignoreExpiration: true,
      secretOrKey: appEnvService.getJwtSecretKeyPasswordExp(),
    });
  }

  static key = 'accessToken-password-expiration';

  async validate(payload: any) {
    return payload.oneTime === 'password-expiration'
      ? { userId: payload.sub, username: payload.username }
      : null;
  }
}
