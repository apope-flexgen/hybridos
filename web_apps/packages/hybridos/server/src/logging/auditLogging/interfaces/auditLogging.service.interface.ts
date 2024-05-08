import { FimsMsg } from '../../../fims/interfaces/fims.interface';
import { User } from '../../../users/dtos/user.dto';
import { AuditLogDTO } from '../dtos/auditLog.dto';

export const AUDIT_LOGGING_SERVICE = 'AuditLoggingService';

export interface IAuditLoggingService {
  postAuditLog(data: AuditLogDTO, user: User): Promise<FimsMsg>;
}
