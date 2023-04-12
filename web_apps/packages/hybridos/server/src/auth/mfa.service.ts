import { Inject, Injectable } from '@nestjs/common'
import { JwtService } from '@nestjs/jwt'
import { authenticator } from 'otplib'
import { toDataURL } from 'qrcode'

import {
    SITE_ADMINS_SERVICE,
    ISiteAdminsService,
} from '../siteAdmins/interfaces/siteAdmin.service.interface'
import { User } from '../../../shared/types/dtos/auth.dto'
import { MfaRequiredException } from './exceptions/mfaRequired.exception'

@Injectable()
export class MfaService {
    constructor(
        @Inject(SITE_ADMINS_SERVICE)
        private readonly siteAdminsService: ISiteAdminsService,
        private readonly jwtService: JwtService
    ) {}
    generateURL(user: User): string {
        const issuer = 'flexgen'

        return authenticator.keyuri(user.username, issuer, user.shared_key)
    }

    async generateQRCode(user: User): Promise<string> {
        const authURL = this.generateURL(user)
        const qrString = await toDataURL(authURL)

        return qrString
    }

    async mfaResponse(user: User) {
        const payload = { username: user.username, oneTime: 'mfa', sub: user.id }
        const accessToken = this.jwtService.sign(payload, {
            expiresIn: '12d',
            secret: 'supersecretkey-oneTimeUse-mfa',
        })

        const qrString = user.mfa_enabled ? null : await this.generateQRCode(user)

        return {
            username: user.username,
            mfaRequired: true,
            oneTimeAccessToken: accessToken,
            qrCode: qrString,
        }
    }

    authenticate(secret: string, totp: string): boolean {
        return authenticator.verify({ token: totp, secret: secret })
    }

    async checkIfSiteMfaEnabled(user: User) {
        const settings = await this.siteAdminsService.find()

        if (settings.password.multi_factor_authentication) {
            throw new MfaRequiredException(user)
        }
    }
}
