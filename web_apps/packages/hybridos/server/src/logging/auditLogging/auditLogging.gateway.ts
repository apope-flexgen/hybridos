import { Inject, UseFilters, UsePipes, ValidationPipe } from '@nestjs/common'
import {
    SubscribeMessage,
    WebSocketGateway,
    WebSocketServer,
} from '@nestjs/websockets'
import { AppExceptionsFilter } from 'src/filters/all-exceptions.filter'
import { Server } from 'ws'
import { SocketMessageBody } from '../../decorators/socketMessageBody.decorator'
import { UserFromSocket } from '../../decorators/userFromSocket.decorator'
import { FimsMsg } from '../../fims/interfaces/fims.interface'
import { AuditLoggingService } from './auditLogging.service'
import { AuditLogDTO } from './dtos/auditLog.dto'
import { AUDIT_LOGGING_SERVICE } from './interfaces/auditLogging.service.interface'

@WebSocketGateway({
    cors: {
        origin: '*',
    },
})
@UseFilters(AppExceptionsFilter)
export class AuditLoggingGateway {
    constructor(
        @Inject(AUDIT_LOGGING_SERVICE)
        private readonly auditLoggingService: AuditLoggingService
    ) {}
    @WebSocketServer()
    server: Server
    @SubscribeMessage('audit-logging')
    @UsePipes(new ValidationPipe({ transform: true }))
    async request(
        @SocketMessageBody() data: AuditLogDTO,
        @UserFromSocket() user
    ): Promise<FimsMsg> {
        return await this.auditLoggingService.postAuditLog(data, user)
    }
}
