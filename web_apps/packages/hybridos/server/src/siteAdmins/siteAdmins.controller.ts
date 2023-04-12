import { Body, Controller, Get, Inject, Post, Req, UseGuards } from '@nestjs/common'
import { ApiOkResponse, ApiTags } from '@nestjs/swagger'
import { RolesGuard } from 'src/auth/guards/roles.guard'
import { DBI_SERVICE } from 'src/dbi/dbi.constants'
import { IDBIService } from 'src/dbi/dbi.interface'
import { UserFromAccessToken } from 'src/decorators/userFromAccessToken.decorator'
import { User } from 'src/users/dtos/user.dto'
import { Roles } from '../../../shared/types/api/Users/Users.types'
import { ApprovedRoles } from '../decorators/roles.decorator'
import { SiteAdminsDto } from './dto/create-siteAdmins.dto'
import { RadiusTestDto } from './dto/radiusTest-siteAdmins.dto'
import {
    SITE_ADMINS_SERVICE,
    ISiteAdminsService,
} from './interfaces/siteAdmin.service.interface'
import { SiteAdminsResponse, RadiusTestResponse } from './responses/siteAdmins.response'

@ApiTags('site-admins')
@Controller('site-admins')
export class SiteAdminsController {
    constructor(
        @Inject(SITE_ADMINS_SERVICE)
        private readonly siteAdminsService: ISiteAdminsService,
        @Inject(DBI_SERVICE)
        private readonly dbiService: IDBIService,
    ) {}
    @ApiOkResponse({ type: SiteAdminsResponse })
    @Get()
    @ApprovedRoles(Roles.Admin, Roles.Developer)
    async find(): Promise<SiteAdminsResponse> {
        return this.siteAdminsService.find()
    }
    @ApiOkResponse({ type: SiteAdminsResponse })
    @Post()
    @ApprovedRoles(Roles.Admin, Roles.Developer)
    async create(
        @Body() createSiteAdminsDto: SiteAdminsDto,
        @UserFromAccessToken() user: User
    ): Promise<SiteAdminsResponse> {
        const response = await this.siteAdminsService.create(createSiteAdminsDto);
        const data = {
            modified_field: 'app settings created', 
            modified_value: true,
        }
        await this.dbiService.postUIConfigAuditLog(data, user);
        return response
    }
    @Post('radius-test')
    @ApprovedRoles(Roles.Admin, Roles.Developer)
    async radiusTest(@Body() radiusSettings: RadiusTestDto): Promise<RadiusTestResponse> {
        const role = await this.siteAdminsService.radiusTest(radiusSettings)
        return role
            ? {message: `Successful radius test with user: ${radiusSettings.username}`}
            : {message: 'Radius test failed'}
    }
}
