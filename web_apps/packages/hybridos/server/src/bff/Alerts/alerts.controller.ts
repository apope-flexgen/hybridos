import {
    Body,
    Controller,
    Get,
    Param,
    Post,
    Query,
    UsePipes,
    ValidationPipe,
} from '@nestjs/common'
import {
    ApiDefaultResponse,
    ApiOkResponse,
    ApiSecurity,
    ApiTags,
} from '@nestjs/swagger'
import { AlertsService } from './alerts.service'
import { ActiveAlertsResponse, ResolveAlertResponse } from './responses/alerts.response'
import { AlertConfigurationsResponse } from './responses/alertConfig.response'
import { AlertsPostResponse } from './responses/alertPost.response'
import { DefaultApiError } from 'src/exceptions/defaultResponse.exception'
import { AlertsRequest } from './dtos/alerts.dto';
import { AlertConfigDTO } from './dtos/alertConfig.dto';
import { UserFromAccessToken } from 'src/decorators/userFromAccessToken.decorator';
import { User } from 'src/users/dtos/user.dto';
import { FimsMsg } from 'src/fims/interfaces/fims.interface'

@ApiTags('alerts')
@ApiSecurity('bearerAuth')
@ApiDefaultResponse({ type: DefaultApiError })
@Controller('alerts')
export class AlertsController {
    constructor(private readonly alertsService: AlertsService) { }
    @Get('active')
    @ApiOkResponse({ type: ActiveAlertsResponse })
    @UsePipes(new ValidationPipe({ transform: true }))
    async alerts(@Query() query: AlertsRequest): Promise<ActiveAlertsResponse> {
        return await this.alertsService.activeAlerts(query)
    }

    @Get('configuration')
    @ApiOkResponse({type: AlertConfigurationsResponse})
    @UsePipes(new ValidationPipe({ transform: true }))
    async alertConfiguration(): Promise<AlertConfigurationsResponse> {
        return await this.alertsService.alertConfigurations()
    }

    @Post('configuration/:alertId')
    @ApiOkResponse({type: AlertsPostResponse})
    async postConfiguration(
        @Body() alertConfigDTO: AlertConfigDTO,
        @Param() alertId: {alertId: string},
        @UserFromAccessToken() user: User,
    ): Promise<AlertsPostResponse> {
        const addAlertConfigResponse = await this.alertsService.updateConfig(alertId.alertId, alertConfigDTO, user.username);
        return addAlertConfigResponse;
    }

    @Post('configuration')
    @ApiOkResponse({type: AlertsPostResponse})
    async updateConfiguration(
        @Body() alertConfigDTO: AlertConfigDTO,
        @UserFromAccessToken() user: User,
    ): Promise<AlertsPostResponse> {
        const addAlertConfigResponse = await this.alertsService.addNewConfig(alertConfigDTO, user.username);
        return addAlertConfigResponse;
    }

    @Post(':id')
    @ApiOkResponse({ type: ResolveAlertResponse })
    async resolveAlert(
        @Param('id') id: string,
        @Body() body: { message: string },
        @UserFromAccessToken() user: User,
    ): Promise<FimsMsg> {
        return await this.alertsService.resolveAlert(
            id,
            body.message,
            user.username,
        );
    }
}
