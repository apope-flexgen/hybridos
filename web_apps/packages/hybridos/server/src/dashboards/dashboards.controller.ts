import { Body, Controller, Get, Inject, Post, Req } from '@nestjs/common'
import { ApiOkResponse, ApiTags } from '@nestjs/swagger';
import { DASHBOARDS_SERVICE } from './dashboards.constants';
import { DashboardsResponse } from './responses';
import { IDashboardsService } from './dashboards.interface';
import { UserFromAccessToken } from 'src/decorators/userFromAccessToken.decorator';
import { User } from 'src/users/dtos/user.dto';

@ApiTags('dashboards')
@Controller()
export class DashboardsController {
    constructor(
        @Inject(DASHBOARDS_SERVICE)
        private readonly dashboardsService: IDashboardsService,
    ) { }
    @ApiOkResponse({ type: DashboardsResponse })
    @Get('/dashboards')
    async getDashboards() {
        return this.dashboardsService.getDashboards()
    }
    @ApiOkResponse({ type: DashboardsResponse })
    @Post('/dashboards')
    async postDashboards(@Body() data, @UserFromAccessToken() user: User) {
        return this.dashboardsService.postDashboards(data, user)
    }
}
