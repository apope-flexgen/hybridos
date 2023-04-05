import { Controller, Get, Param } from '@nestjs/common'
import { ApprovedRoles } from '../decorators/roles.decorator'
import { Roles } from '../../../shared/types/api/Users/Users.types'

import { RestService } from './rest.service'

@Controller('rest')
export class RestController {
    constructor(private readonly restService: RestService) {}

    @Get(':endpoint(*)')
    @ApprovedRoles(Roles.Rest)
    async get(@Param() params): Promise<any> {
        return await this.restService.getAggregatedEndpoint(`/${params.endpoint}`)
    }
}
