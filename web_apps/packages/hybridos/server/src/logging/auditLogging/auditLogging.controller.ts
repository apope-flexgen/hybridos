import { Body, Controller, Inject, Post } from '@nestjs/common'
import { ApiDefaultResponse, ApiSecurity, ApiTags } from '@nestjs/swagger'
import { UserFromAccessToken } from '../../decorators/userFromAccessToken.decorator'
import { FimsMsg } from '../../fims/interfaces/fims.interface'
import { AuditLoggingService } from './auditLogging.service'
import { AuditLogDTO } from './dtos/auditLog.dto'
import { AUDIT_LOGGING_SERVICE } from './interfaces/auditLogging.service.interface'
import { DefaultApiError } from 'src/exceptions/defaultResponse.exception'

@ApiTags('audit-logging')
@ApiSecurity('bearerAuth')
@ApiDefaultResponse({ type: DefaultApiError })
@Controller('audit-logging')
export class AuditLoggingController {
    constructor(
        @Inject(AUDIT_LOGGING_SERVICE)
        private readonly auditLoggingService: AuditLoggingService
    ) {}

    @Post()
    async logData(
        @UserFromAccessToken() user,
        @Body() data: AuditLogDTO
    ): Promise<FimsMsg> {
        return await this.auditLoggingService.postAuditLog(data, user)
    }
}
