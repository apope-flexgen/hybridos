import { Body, Controller, Get, Inject, Post, UseGuards } from '@nestjs/common'
import {
    ApiDefaultResponse,
    ApiOkResponse,
    ApiSecurity,
    ApiTags,
} from '@nestjs/swagger'
import { RolesGuard } from 'src/auth/guards/roles.guard'
import { UserFromAccessToken } from 'src/decorators/userFromAccessToken.decorator'
import { DefaultApiError } from 'src/exceptions/defaultResponse.exception'
import { User } from 'src/users/dtos/user.dto'
import { Roles } from '../../../shared/types/api/Users/Users.types'
import { ApprovedRoles } from '../decorators/roles.decorator'
import {
    AUDIT_LOGGING_SERVICE,
    IAuditLoggingService,
} from '../logging/auditLogging/interfaces/auditLogging.service.interface'
import { SiteAdminsDto } from './dto/create-siteAdmins.dto'
import { RadiusTestDto } from './dto/radiusTest-siteAdmins.dto'
import {
    SITE_ADMINS_SERVICE,
    ISiteAdminsService,
} from './interfaces/siteAdmin.service.interface'
import {
    SiteAdminsResponse,
    RadiusTestResponse,
} from './responses/siteAdmins.response'

@UseGuards(RolesGuard)
@ApiTags('site-admins')
@ApiSecurity('bearerAuth')
@ApiDefaultResponse({ type: DefaultApiError })
@Controller('site-admins')
export class SiteAdminsController {
    constructor(
        @Inject(SITE_ADMINS_SERVICE)
        private readonly siteAdminsService: ISiteAdminsService,
        @Inject(AUDIT_LOGGING_SERVICE)
        private readonly auditLoggingService: IAuditLoggingService
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
        const response = await this.siteAdminsService.create(
            createSiteAdminsDto
        )
        const data = {
            modified_field: 'app settings created',
            modified_value: true,
        }
        await this.auditLoggingService.postAuditLog(data, user)
        return response
    }

    @Post('radius-test')
    @ApprovedRoles(Roles.Admin, Roles.Developer)
    @ApiOkResponse({ type: RadiusTestResponse })
    async radiusTest(
        @Body() radiusSettings: RadiusTestDto
    ): Promise<RadiusTestResponse> {
        const role = await this.siteAdminsService.radiusTest(radiusSettings)
        return role
            ? {
                  message: `Successful radius test with user: ${radiusSettings.username}`,
              }
            : { message: 'Radius test failed' }
    }
}
