import {
  Body,
  Controller,
  Delete,
  Get,
  Param,
  Post,
  Query,
  UsePipes,
  ValidationPipe,
} from '@nestjs/common';
import { ApiDefaultResponse, ApiOkResponse, ApiSecurity, ApiTags } from '@nestjs/swagger';
import { AlertsService } from './alerts.service';
import {
  ActiveAlertsResponse,
  ResolveAlertResponse,
  ResolvedAlertsResponse,
} from './responses/alerts.response';
import { AlertConfigurationsResponse } from './responses/alertConfig.response';
import { AlertsPostResponse } from './responses/alertPost.response';
import { DefaultApiError } from 'src/exceptions/defaultResponse.exception';
import { AlertsRequest } from './dtos/alerts.dto';
import { OrganizationsDTO } from './dtos/organizations.dto';
import { AlertConfigDTO } from './dtos/alertConfig.dto';
import { UserFromAccessToken } from 'src/decorators/userFromAccessToken.decorator';
import { User } from 'src/users/dtos/user.dto';
import { OrganizationsResponse } from './responses/organizations.response';

@ApiTags('alerts')
@ApiSecurity('bearerAuth')
@ApiDefaultResponse({ type: DefaultApiError })
@Controller('alerts')
export class AlertsController {
  constructor(private readonly alertsService: AlertsService) {}
  @Get('active')
  @ApiOkResponse({ type: ActiveAlertsResponse })
  @UsePipes(new ValidationPipe({ transform: true }))
  async alerts(@Query() query: AlertsRequest): Promise<ActiveAlertsResponse> {
    return await this.alertsService.activeAlerts(query);
  }

  @Get('configuration')
  @ApiOkResponse({ type: AlertConfigurationsResponse })
  @UsePipes(new ValidationPipe({ transform: true }))
  async alertConfiguration(): Promise<AlertConfigurationsResponse> {
    return await this.alertsService.alertConfigurations();
  }

  @Post('configuration/:alertId')
  @ApiOkResponse({ type: AlertsPostResponse })
  async editConfiguration(
    @Body() alertConfigDTO: AlertConfigDTO,
    @Param() alertId: { alertId: string },
    @UserFromAccessToken() user: User,
  ): Promise<AlertsPostResponse> {
    const addAlertConfigResponse = await this.alertsService.updateConfig(
      alertId.alertId,
      alertConfigDTO,
      user.username,
    );
    return addAlertConfigResponse;
  }

  @Post('configuration')
  @ApiOkResponse({ type: AlertsPostResponse })
  async updateConfiguration(
    @Body() alertConfigDTO: AlertConfigDTO,
    @UserFromAccessToken() user: User,
  ): Promise<AlertsPostResponse> {
    const addAlertConfigResponse = await this.alertsService.addNewConfig(
      alertConfigDTO,
      user.username,
    );
    return addAlertConfigResponse;
  }

  @Post('organizations')
  @ApiOkResponse({ type: ResolveAlertResponse })
  async editOrganizations(
    @Body() newOrganizations: OrganizationsDTO,
    @UserFromAccessToken() user: User,
  ): Promise<ResolveAlertResponse> {
    return await this.alertsService.editOrganizations(newOrganizations, user.username);
  }

  @Get('organizations')
  @ApiOkResponse({ type: OrganizationsResponse })
  async getOrganizations(): Promise<OrganizationsResponse> {
    return await this.alertsService.getOrganizations();
  }

  @Delete('organizations/:id')
  @ApiOkResponse({ type: ResolveAlertResponse })
  async deleteOrganization(
    @Param('id') id: string,
    @UserFromAccessToken() user: User,
  ): Promise<ResolveAlertResponse> {
    return await this.alertsService.deleteOrganization(id, user.username);
  }

  @Post(':id')
  @ApiOkResponse({ type: ResolveAlertResponse })
  async resolveAlert(
    @Param('id') id: string,
    @Body() body: { message: string },
    @UserFromAccessToken() user: User,
  ): Promise<ResolveAlertResponse> {
    return await this.alertsService.resolveAlert(id, body.message, user.username);
  }

  @Get('resolved')
  @ApiOkResponse({ type: ResolvedAlertsResponse })
  @UsePipes(new ValidationPipe({ transform: true }))
  async resolvedAlerts(@Query() query: AlertsRequest): Promise<ResolvedAlertsResponse> {
    return await this.alertsService.resolvedAlerts(query);
  }
}
