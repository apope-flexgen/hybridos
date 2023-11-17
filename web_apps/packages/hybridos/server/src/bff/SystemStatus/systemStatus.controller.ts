import {
    Controller,
    Get,
    UseGuards,
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
import { ServiceStatusReponse } from './dto/serviceStatusResponse.dto';

@UseGuards(RolesGuard)
@ApiTags('system-status')
@ApiSecurity('bearerAuth')
@ApiDefaultResponse({type: DefaultApiError})
@Controller('system-status')
export class SystemStatusController {
    constructor(private readonly systemStatusService: SystemStatusService) {}
    // NOTE: Service method called below does not currently return data
    // backend cops service will need to be updated
    @Get()
    @ApiOkResponse({type: ServiceStatusReponse})
    @ApprovedRoles(Roles.Admin, Roles.Developer)
    async getSystemStatus(): Promise<ServiceStatusReponse> {
        const systemStatusResponse = await this.systemStatusService.getSystemStatus();
        return systemStatusResponse;
    }
}
