import { Injectable } from '@nestjs/common'
import { PassportStrategy } from '@nestjs/passport'
import { Strategy, ExtractJwt } from 'passport-jwt'
import { AppEnvService } from 'src/environment/appEnv.service'

@Injectable()
export class AccessTokenPassExpStrategy extends PassportStrategy(Strategy, 'accessToken-password-expiration') {
    constructor(appEnvService: AppEnvService) {
        super({
            jwtFromRequest: ExtractJwt.fromAuthHeaderAsBearerToken(),
            ignoreExpiration: true,
            secretOrKey: appEnvService.getJwtSecretKeyPasswordExp()
        })
    }

    static key = 'accessToken-password-expiration'

    async validate(payload: any) {
        return payload.oneTime === 'password-expiration'
            ? { userId: payload.sub, username: payload.username }
            : null
    }
}
