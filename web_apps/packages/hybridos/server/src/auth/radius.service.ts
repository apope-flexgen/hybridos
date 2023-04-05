import { Inject, Injectable } from '@nestjs/common'

import {
    APP_SETTINGS_SERVICE,
    IAppSettingsService,
} from '../appSettings/interfaces/appSetting.service.interface'
import { AuthRadius } from '../radius/authRadius'

@Injectable()
export class RadiusService {
    constructor(
        @Inject(APP_SETTINGS_SERVICE)
        private readonly appSettingsService: IAppSettingsService
    ) {}
    async authenticate(username: string, password: string): Promise<string> {
        const site = await this.appSettingsService.find()

        if (!site.radius.is_enabled) {
            return null
        }

        const port = parseInt(site.radius.port)
        const authRadius = new AuthRadius(
            site.radius.ip_address,
            site.radius.secret_phrase,
            port,
            site.radius.wait_time
        )

        return new Promise((resolve) => {
            authRadius
                .authenticate(username, password)
                .onAccept((decodedPacket) => {
                    const role = authRadius.getAttributeRole(decodedPacket)
                    resolve(role)
                })
                .onReject(() => {
                    resolve(null)
                })
                .onError(() => {
                    resolve(null)
                })
                .onTimeout(() => {
                    resolve(null)
                })
        })
    }
}
