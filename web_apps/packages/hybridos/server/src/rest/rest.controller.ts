import { Controller, Get, HttpCode, Param, Put, Req, UseFilters, UseGuards } from '@nestjs/common'
import { ApprovedRoles } from '../decorators/roles.decorator'
import { Roles } from '../../../shared/types/api/Users/Users.types'

import { RestService } from './rest.service'
import { BasicAccessGuard } from 'src/auth/guards/basicAccessAuth.guard'
import { UserNotFoundFilter } from 'src/users/filters/userNotFound.filter'
import { BasicAuth } from 'src/decorators/basicAuth.decorator'
import { RolesGuard } from 'src/auth/guards/roles.guard'
import { Throttle } from '@nestjs/throttler'
import { REST_API_LIMIT, REST_API_TTL } from 'src/environment/appEnv.constants'
import { ApiResponse } from '@nestjs/swagger'
import { RestSetValueResponse } from './responses/restsetvalue.responses'

@UseGuards(BasicAccessGuard, RolesGuard)
@Controller('rest')
@Throttle(REST_API_LIMIT, REST_API_TTL)
export class RestController {
    constructor(private readonly restService: RestService) {}

    @Get(':endpoint(*)')
    @BasicAuth()
    @ApprovedRoles(Roles.Rest)
    @UseFilters(UserNotFoundFilter)
    async get(@Param() params, @Req() request): Promise<any> {
        return await this.restService.getAggregatedEndpoint(`/${params.endpoint}`, request.user)
    }

    @Put(':endpoint(*)/:value')
    @BasicAuth()
    @ApiResponse({ type: RestSetValueResponse })
    @ApprovedRoles(Roles.Rest)
    @HttpCode(202)
    @UseFilters(UserNotFoundFilter)
    async put(@Param() params, @Req() request): Promise<RestSetValueResponse> {
        return await this.restService.setValue(`/${params.endpoint}`, params.value, request.user)
    }
}
