import { Inject, Injectable, OnApplicationBootstrap } from '@nestjs/common'
import { AppEnvService } from '../environment/appEnv.service'
import { FIMS_SERVICE, IFimsService } from '../fims/interfaces/fims.interface'
import { PermissionsService } from '../permissions/permissions.service'
import { Roles } from '../../../shared/types/api/Users/Users.types'

const PERMISSION_LEVEL = 'read'
const USER_ROLE = Roles.Rest

@Injectable()
export class RestService implements OnApplicationBootstrap {
    aggregatedEndpoints: any

    constructor(
        private readonly appEnvService: AppEnvService,
        @Inject(FIMS_SERVICE) private readonly fimsService: IFimsService,
        private readonly permissionsService: PermissionsService
    ) {}

    async onApplicationBootstrap() {
        this.aggregatedEndpoints = this.appEnvService.getAggregatedEndpoints()
        console.log('aggregated: ', this.aggregatedEndpoints)
    }

    async getAggregatedEndpoint(topLevelEndpoint: string) {
        if (!this.aggregatedEndpoints[topLevelEndpoint]) {
            return 'aggregated endpoint does not exist'
        }

        const result = await Promise.all(
            this.aggregatedEndpoints[topLevelEndpoint]
                .filter((endpoint) => {
                    const fullURI = `${topLevelEndpoint}/${endpoint}`

                    const sufficientPermissions = this.permissionsService.ConfirmRoleAccess(
                        USER_ROLE,
                        PERMISSION_LEVEL,
                        fullURI
                    )

                    return sufficientPermissions
                })
                .map(async (endpoint) => {
                    const fullURI = `${topLevelEndpoint}/${endpoint}`

                    const response = await this.fimsService.get(fullURI)
                    return {
                        [endpoint]: response.body,
                    }
                })
        )

        if (result.length === 0) return {}

        const formatted = result.reduce((prev, cur) => {
            return { ...prev, ...cur }
        })

        return formatted
    }
}
