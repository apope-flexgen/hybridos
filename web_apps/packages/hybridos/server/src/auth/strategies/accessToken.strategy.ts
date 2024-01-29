import { Inject, Injectable } from '@nestjs/common';
import { PassportStrategy } from '@nestjs/passport';
import { Strategy, ExtractJwt } from 'passport-jwt';
import { IValidJWTService, VALID_JWT_SERVICE } from '../interfaces/validJWT.service.interface';
import { APP_ENV_SERVICE, IAppEnvService } from 'src/environment/appEnv.interface';

@Injectable()
export class AccessTokenStrategy extends PassportStrategy(Strategy) {
  constructor(
    @Inject(VALID_JWT_SERVICE)
    private readonly validJwtService: IValidJWTService,
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
    const accessToken = this.validJwtService.extractAccessTokenFromRequest(req);
    return this.validJwtService.containsAccessToken(accessToken)
      ? { role: payload.role, username: payload.sub }
      : null;
  }
}
