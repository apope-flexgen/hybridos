import { Inject, Injectable, UnauthorizedException } from '@nestjs/common'
import { PassportStrategy } from '@nestjs/passport'
import { Strategy } from 'passport-local'
import { User } from '../../../../shared/types/dtos/auth.dto'
import { AUTH_SERVICE, IAuthService } from '../interfaces/auth.service.interface'
import { IMfaService, MFA_SERVICE } from '../interfaces/mfa.service.interface'
import { IPassExpService, PASS_EXP_SERVICE } from '../interfaces/passExp.service.interface'
import { ISiteAdminsService, SITE_ADMINS_SERVICE } from '../../siteAdmins/interfaces/siteAdmin.service.interface'

@Injectable()
export class LocalStrategy extends PassportStrategy(Strategy) {
    constructor(
        @Inject(AUTH_SERVICE)
        private readonly authService: IAuthService,
        @Inject(MFA_SERVICE)
        private readonly mfaService: IMfaService,
        @Inject(PASS_EXP_SERVICE)
        private readonly passExpService: IPassExpService,
        @Inject(SITE_ADMINS_SERVICE)
        private readonly siteAdminsService: ISiteAdminsService
    ) {
        super()
    }
    async validate(username: string, password: string): Promise<User> {
        // get site admin settings
        const siteAdmins = await this.siteAdminsService.find()
        const { is_enabled, ip_address, port, secret_phrase, wait_time, is_local_auth_disabled } = siteAdmins.radius

        // only authenticate if radius is enabled
        if (!is_local_auth_disabled) {
            const user = await this.authService.validateUser(username, password)
            if (!user) {
                throw new UnauthorizedException()
            }

            await this.passExpService.checkIfPasswordExpired(user)
            await this.mfaService.checkIfSiteMfaEnabled(user)

            return {
                username: user.username,
                role: user.role,
            }
        }
    }
}
