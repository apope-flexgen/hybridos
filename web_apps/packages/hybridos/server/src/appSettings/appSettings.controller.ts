import { Body, Controller, Get, Inject, Post, Req } from '@nestjs/common'
import { ApiOkResponse, ApiTags } from '@nestjs/swagger'
import { DBI_SERVICE } from 'src/dbi/dbi.constants'
import { IDBIService, URIs } from 'src/dbi/dbi.interface'
import { UserFromAccessToken } from 'src/decorators/userFromAccessToken.decorator'
import { User } from 'src/users/dtos/user.dto'
import { Roles } from '../../../shared/types/api/Users/Users.types'
import { ApprovedRoles } from '../decorators/roles.decorator'
import { CreateAppSettingsDto } from './dto/create-appSettings.dto'
import { RadiusTestDto } from './dto/radiusTest-appSettings.dto'
import {
    APP_SETTINGS_SERVICE,
    IAppSettingsService,
} from './interfaces/appSetting.service.interface'
import { AppSettingsResponse, RadiusTestResponse } from './responses/appSettings.response'

@ApiTags('app-settings')
@Controller('app-settings')
export class AppSettingsController {
    constructor(
        @Inject(APP_SETTINGS_SERVICE)
        private readonly appSettingsService: IAppSettingsService,
        @Inject(DBI_SERVICE)
        private readonly dbiService: IDBIService,
    ) {}
    @ApiOkResponse({ type: AppSettingsResponse })
    @Get()
    @ApprovedRoles(Roles.Admin, Roles.Developer)
    async find(): Promise<AppSettingsResponse> {
        return this.appSettingsService.find()
    }
    @ApiOkResponse({ type: AppSettingsResponse })
    @Post()
    @ApprovedRoles(Roles.Admin, Roles.Developer)
    async create(
        @Body() createAppSettingsDto: CreateAppSettingsDto,
        @UserFromAccessToken() user: User
    ): Promise<AppSettingsResponse> {
        const response = await this.appSettingsService.create(createAppSettingsDto);
        const data = {
            'modified_field': 'app settings created',
            'modified_value': true,
            'username': user.username,
            'userrole': user.role,
        }
        await this.dbiService.postUIConfigAuditLog(data);
        return response
    }
    @Post('radius-test')
    @ApprovedRoles(Roles.Admin, Roles.Developer)
    async radiusTest(@Body() radiusSettings: RadiusTestDto): Promise<RadiusTestResponse> {
        const role = await this.appSettingsService.radiusTest(radiusSettings)
        return role
            ? {message: `Successful radius test with user: ${radiusSettings.username}`}
            : {message: 'Radius test failed'}
    }
}
