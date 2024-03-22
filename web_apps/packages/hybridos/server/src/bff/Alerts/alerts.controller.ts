import {
    Controller,
    Get,
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
import { ActiveAlertsResponse } from './responses/alerts.response'
import { DefaultApiError } from 'src/exceptions/defaultResponse.exception'
import { ActiveAlertsRequest } from './dtos/alerts.dto';

@ApiTags('alerts')
@ApiSecurity('bearerAuth')
@ApiDefaultResponse({type: DefaultApiError})
@Controller('alerts')
export class AlertsController {
    constructor(private readonly alertsService: AlertsService) {}
    @Get('active')
    @ApiOkResponse({type: ActiveAlertsResponse})
    @UsePipes(new ValidationPipe({ transform: true }))
    async alerts(@Query() query: ActiveAlertsRequest): Promise<ActiveAlertsResponse> {
        return await this.alertsService.activeAlerts(query)
    }
}
