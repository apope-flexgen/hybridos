import { Inject, Injectable } from '@nestjs/common'
import { JwtService } from '@nestjs/jwt'

import {
    SITE_ADMINS_SERVICE,
    ISiteAdminsService,
} from '../siteAdmins/interfaces/siteAdmin.service.interface'
import { User } from '../../../shared/types/dtos/auth.dto'
import { PasswordExpiredException } from './exceptions/passwordExpired.exception'

@Injectable()
export class PassExpService {
    constructor(
        private readonly jwtService: JwtService,
        @Inject(SITE_ADMINS_SERVICE)
        private readonly siteAdminsService: ISiteAdminsService
    ) {}

    DAY_MS = 86400000
    HOUR_MS = 3600000
    MIN_MS = 60000

    parseInterval(interval: string): number {
        const numString = interval.slice(0, interval.length - 1)
        const cleanNumber = parseInt(numString)
        const letter = interval.slice(interval.length - 1)

        let intervalMilliseconds: number
        switch (letter) {
            case 'd':
                intervalMilliseconds = cleanNumber * this.DAY_MS
                break
            case 'h':
                intervalMilliseconds = cleanNumber * this.HOUR_MS
                break
            case 'm':
                intervalMilliseconds = cleanNumber * this.MIN_MS
                break
            default:
                break
        }
        return intervalMilliseconds
    }

    passwordExpired(updatedDate: Date, timeInterval: string): boolean {
        const rxp = new RegExp('^[1-9]\\d*[dmh]$')
        
        if (!rxp.test(timeInterval)) {
            throw new Error('invalid password expiration time interval')
        }

        const intervalMilliseconds = this.parseInterval(timeInterval)

        const timeElapsed = Date.now() - updatedDate.getTime()

        return timeElapsed > intervalMilliseconds
    }

    async passExpResponse(user: User) {
        const payload = {
            username: user.username,
            oneTime: 'password-expiration',
            sub: user.id,
        }
        const accessToken = this.jwtService.sign(payload, {
            expiresIn: '12d',
            secret: 'supersecretkey-oneTimeUse-password-expiration',
        })

        return {
            username: user.username,
            passwordExpired: true,
            oneTimeAccessToken: accessToken,
        }
    }

    async checkIfPasswordExpired(user: User) {
        const settings = await this.siteAdminsService.find()

        if (settings.password.password_expiration) {
            // check if password is expired
            const passwordExpired = this.passwordExpired(
                user.pwdUpdatedDate,
                settings.password.password_expiration_interval
            )
            // if yes, throw custom exception
            if (passwordExpired) {
                throw new PasswordExpiredException(user)
            }
        }
    }
}
