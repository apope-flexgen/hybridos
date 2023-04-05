import { Inject, Injectable } from '@nestjs/common'
import { PassportStrategy } from '@nestjs/passport'
import { Strategy } from 'passport-local'

import { IUsersService, USERS_SERVICE } from '../../users/interfaces/users.service.interface'
import { IncorrectTotpCodeException } from '../exceptions/incorrectTotpCode.exception'
import { IMfaService, MFA_SERVICE } from '../interfaces/mfa.service.interface'

@Injectable()
export class TotpStrategy extends PassportStrategy(Strategy, 'totp') {
    constructor(
        @Inject(USERS_SERVICE) private readonly usersService: IUsersService,
        @Inject(MFA_SERVICE)
        private readonly mfaService: IMfaService
    ) {
        super({ passwordField: 'totp' })
    }
    static key = 'totp'
    async validate(username: string, totp: string): Promise<any> {
        const user = await this.usersService.readByUsername(username)
        const correctCode = this.mfaService.authenticate(user.shared_key, totp)

        // this leads to 401 w/ incorrect totp code message
        if (!correctCode) {
            throw new IncorrectTotpCodeException()
        }
        return user

        // return correctCode ? user : null; // this leads to a 401 w/ Unauthorized message
    }
}
