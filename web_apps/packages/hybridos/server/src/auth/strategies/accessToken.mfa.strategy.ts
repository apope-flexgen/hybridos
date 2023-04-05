import { Inject, Injectable } from '@nestjs/common'
import { PassportStrategy } from '@nestjs/passport'
import { Strategy, ExtractJwt } from 'passport-jwt'

import { IValidJWTService, VALID_JWT_SERVICE } from '../interfaces/validJWT.service.interface'

@Injectable()
export class AccessTokenMfaStrategy extends PassportStrategy(Strategy, 'accessToken-mfa') {
    constructor(
        @Inject(VALID_JWT_SERVICE)
        private readonly validJwtService: IValidJWTService
    ) {
        super({
            jwtFromRequest: ExtractJwt.fromAuthHeaderAsBearerToken(),
            ignoreExpiration: false,
            secretOrKey: 'supersecretkey-oneTimeUse-mfa',
        })
    }

    static key = 'accessToken-mfa'

    async validate(payload: any) {
        return payload.oneTime === 'mfa'
            ? { userId: payload.sub, username: payload.username }
            : null
    }
}
