import {
    Controller,
    Get,
    UseGuards,
    Param,
    Put,
} from '@nestjs/common'
import {
    ApiDefaultResponse,
    ApiOkResponse,
    ApiSecurity,
    ApiTags,
} from '@nestjs/swagger'
import { DefaultApiError } from 'src/exceptions/defaultResponse.exception';
import { SystemStatusService } from './systemStatus.service'
import { ApprovedRoles } from '../../decorators/roles.decorator';
import { Roles } from '../../../../shared/types/api/Users/Users.types';
import { RolesGuard } from '../../auth/guards/roles.guard'
import { ServiceStatusResponse } from './dto/serviceStatusResponse.dto';
import { FimsMsg } from 'src/fims/responses/fimsMsg.response';
import { ServiceName } from './params/serviceName.params';
@UseGuards(RolesGuard)
@ApiTags('system-status')
@ApiSecurity('bearerAuth')
@ApiDefaultResponse({type: DefaultApiError})
@Controller('system-status')
export class SystemStatusController {
    constructor(private readonly systemStatusService: SystemStatusService) {}
    @Get()
    @ApiOkResponse({type: ServiceStatusResponse})
    @ApprovedRoles(Roles.Admin, Roles.Developer)
    async getSystemStatus(): Promise<ServiceStatusResponse[]> {
        const systemStatusResponse = await this.systemStatusService.getSystemStatus();
        return systemStatusResponse;
    }

    @Put('/:serviceName/start')
    @ApiOkResponse({type: FimsMsg})
    @ApprovedRoles(Roles.Admin, Roles.Developer)
    async startService(
        @Param() serviceName: ServiceName,
    ): Promise<FimsMsg> {
        const serviceStartResponse = await this.systemStatusService.doServiceAction(serviceName.serviceName, 'start');
        return serviceStartResponse;
    }

    @Put('/:serviceName/stop')
    @ApiOkResponse({type: FimsMsg})
    @ApprovedRoles(Roles.Admin, Roles.Developer)
    async stopService(
        @Param() serviceName: ServiceName,
    ): Promise<FimsMsg> {
        const serviceStartResponse = await this.systemStatusService.doServiceAction(serviceName.serviceName, 'stop');
        return serviceStartResponse;
    }

    @Put('/:serviceName/restart')
    @ApiOkResponse({type: FimsMsg})
    @ApprovedRoles(Roles.Admin, Roles.Developer)
    async restartService(
        @Param() serviceName: ServiceName,
    ): Promise<FimsMsg> {
        const serviceStartResponse = await this.systemStatusService.doServiceAction(serviceName.serviceName, 'restart');
        return serviceStartResponse;
    }
}
