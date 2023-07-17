import { Injectable } from '@nestjs/common'
import { PassportStrategy } from '@nestjs/passport'
import { Strategy, ExtractJwt } from 'passport-jwt'
import { AppEnvService } from 'src/environment/appEnv.service'

@Injectable()
export class AccessTokenMfaStrategy extends PassportStrategy(Strategy, 'accessToken-mfa') {
    constructor(appEnvService: AppEnvService) {
        super({
            jwtFromRequest: ExtractJwt.fromAuthHeaderAsBearerToken(),
            ignoreExpiration: false,
            secretOrKey: appEnvService.getJwtSecretKeyMFA(),
        })
    }

    static key = 'accessToken-mfa'

    async validate(payload: any) {
        return payload.oneTime === 'mfa'
            ? { userId: payload.sub, username: payload.username }
            : null
    }
}
