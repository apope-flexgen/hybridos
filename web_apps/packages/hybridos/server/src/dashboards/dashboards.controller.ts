import { Body, Controller, Get, Inject, Post, Req } from '@nestjs/common'
import {
    ApiDefaultResponse,
    ApiOkResponse,
    ApiSecurity,
    ApiTags,
} from '@nestjs/swagger'
import { DASHBOARDS_SERVICE } from './dashboards.constants'
import { DashboardsResponse } from './responses'
import { IDashboardsService } from './dashboards.interface'
import { UserFromAccessToken } from 'src/decorators/userFromAccessToken.decorator'
import { User } from 'src/users/dtos/user.dto'
import { DashboardCard } from './dtos/dashboard.dto'
import { DefaultApiError } from '../exceptions/defaultResponse.exception'

@ApiTags('dashboards')
@ApiSecurity('bearerAuth')
@ApiDefaultResponse({ type: DefaultApiError })
@Controller()
export class DashboardsController {
    constructor(
        @Inject(DASHBOARDS_SERVICE)
        private readonly dashboardsService: IDashboardsService
    ) {}
    @ApiOkResponse({ type: DashboardsResponse })
    @Get('/dashboards')
    async getDashboards() {
        return this.dashboardsService.getDashboards()
    }
    @ApiOkResponse({ type: DashboardsResponse })
    @Post('/dashboards')
    async postDashboards(@Body() data: DashboardCard) {
        return this.dashboardsService.postDashboards(data)
    }
}
