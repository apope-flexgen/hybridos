import { Module } from '@nestjs/common'
import { DBIModule } from '../../dbi/dbi.module'
import { AuditLoggingController } from './auditLogging.controller'
import { AuditLoggingGateway } from './auditLogging.gateway'
import { AuditLoggingService } from './auditLogging.service'
import { AUDIT_LOGGING_SERVICE } from './interfaces/auditLogging.service.interface'

@Module({
    imports: [DBIModule],
    controllers: [AuditLoggingController],
    providers: [
        { provide: AUDIT_LOGGING_SERVICE, useClass: AuditLoggingService },
        AuditLoggingGateway,
    ],
    exports: [
        {
            provide: AUDIT_LOGGING_SERVICE,
            useClass: AuditLoggingService,
        },
    ],
})
export class AuditLoggingModule {}
