import { Inject, Injectable, UnauthorizedException } from '@nestjs/common'
import { PassportStrategy } from '@nestjs/passport'
import { BasicStrategy } from 'passport-http';
import { User } from '../../../../shared/types/dtos/auth.dto'
import { AUTH_SERVICE, IAuthService } from '../interfaces/auth.service.interface'

@Injectable()
export class BasicAccessStrategy extends PassportStrategy(BasicStrategy, 'basic') {
    constructor(
        @Inject(AUTH_SERVICE)
        private readonly authService: IAuthService,
    ) {
        super()
    }
    async validate(username: string, password: string): Promise<User> {
        try {
            const user = await this.authService.validateUser(username, password)
            if (!user) {
                throw new UnauthorizedException()
            }
            return {
                username: user.username,
                role: user.role,
            }
        } catch (e) {
            throw new UnauthorizedException()
        }
    }
}
