import { Inject, Injectable } from '@nestjs/common'
import { PassportStrategy } from '@nestjs/passport'
import { Strategy, ExtractJwt } from 'passport-jwt'

import { IValidJWTService, VALID_JWT_SERVICE } from '../interfaces/validJWT.service.interface'

@Injectable()
export class AccessTokenPassExpStrategy extends PassportStrategy(Strategy, 'accessToken-password-expiration') {
    constructor(
        @Inject(VALID_JWT_SERVICE)
        private readonly validJwtService: IValidJWTService
    ) {
        super({
            jwtFromRequest: ExtractJwt.fromAuthHeaderAsBearerToken(),
            ignoreExpiration: true,
            secretOrKey: 'supersecretkey-oneTimeUse-password-expiration',
        })
    }

    static key = 'accessToken-password-expiration'

    async validate(payload: any) {
        return payload.oneTime === 'password-expiration'
            ? { userId: payload.sub, username: payload.username }
            : null
    }
}
