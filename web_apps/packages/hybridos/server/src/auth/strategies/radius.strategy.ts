import { Inject, Injectable } from '@nestjs/common'
import { PassportStrategy } from '@nestjs/passport'
import { Strategy } from 'passport-local'

import { IRadiusService, RADIUS_SERVICE } from '../interfaces/radius.service.interface'
import { IAppSettingsService, APP_SETTINGS_SERVICE } from '../../appSettings/interfaces/appSetting.service.interface'

@Injectable()
export class RadiusStrategy extends PassportStrategy(Strategy, 'radius') {
    constructor(
        @Inject(RADIUS_SERVICE)
        private readonly radiusService: IRadiusService,
        @Inject(APP_SETTINGS_SERVICE)
        private readonly appSettingService: IAppSettingsService
    ) {
        super()
    }
    static key = 'radius'
    async validate(username: string, password: string): Promise<any> {
        // get site admin settings
        const appSettings = await this.appSettingService.find()
        const { is_enabled, ip_address, port, secret_phrase, wait_time, is_local_auth_disabled } = appSettings.radius

        // only authenticate if radius is enabled
        if (is_enabled) {
            const role = await this.radiusService.authenticate(username, password)
            if (!role) {
                return null
            }
    
            return {
                username: username,
                role: role,
            }
        }
    }
}
