import { Injectable } from '@nestjs/common'
import { PassportStrategy } from '@nestjs/passport'
import { Strategy } from 'passport-jwt'
import { RefreshTokenService } from '../refreshTokenService'

@Injectable()
export class RefreshTokenStrategy extends PassportStrategy(Strategy, 'refresh-token') {
    constructor(private readonly refreshTokenService: RefreshTokenService) {
        super({
            jwtFromRequest: refreshTokenService.extractRefreshTokenFromRequest,
            ignoreExpiration: false,
            secretOrKey: 'supersecretkey',
            passReqToCallback: true,
        })
    }

    async validate(req, payload: any) {
        const refreshToken = this.refreshTokenService.extractRefreshTokenFromRequest(req)
        return this.refreshTokenService.containsRefreshToken(refreshToken)
            ? { ...payload, refreshToken }
            : null
    }
}
