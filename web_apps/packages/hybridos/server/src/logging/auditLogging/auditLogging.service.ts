import { Inject, Injectable } from '@nestjs/common'
import { User } from 'shared/types/dtos/auth.dto'
import { DBI_SERVICE, DBI_URIs, IDBIService } from '../../dbi/dbi.interface'
import { FimsMsg } from '../../fims/interfaces/fims.interface'
import { AuditLog } from './auditLogging.types'
import { AuditLogDTO } from './dtos/auditLog.dto'
import { IAuditLoggingService } from './interfaces/auditLogging.service.interface'

@Injectable()
export class AuditLoggingService implements IAuditLoggingService {
    constructor(
        @Inject(DBI_SERVICE) private readonly dbiService: IDBIService
    ) {}

    postAuditLog = async (data: AuditLogDTO, user: User): Promise<FimsMsg> => {
        const timestamp = Date.now()

        const auditLog: AuditLog = {
            username: user.username,
            userrole: user.role,
            created: timestamp,
            modified_field: data.modified_field,
            modified_value: data.modified_value,
            ...data.extraFields,
        }

        return await this.dbiService.postToDBI(
            `${DBI_URIs.AUDIT_LOG}${timestamp}`,
            auditLog
        )
    }
}
