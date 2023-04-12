import { Inject, Injectable } from '@nestjs/common'

import {
    SITE_ADMINS_SERVICE,
    ISiteAdminsService,
} from '../siteAdmins/interfaces/siteAdmin.service.interface'
import { AuthRadius } from '../radius/authRadius'

@Injectable()
export class RadiusService {
    constructor(
        @Inject(SITE_ADMINS_SERVICE)
        private readonly siteAdminsService: ISiteAdminsService
    ) {}
    async authenticate(username: string, password: string): Promise<string> {
        const site = await this.siteAdminsService.find()

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
