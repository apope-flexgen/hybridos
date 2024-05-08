import { Global, Module } from '@nestjs/common';
import { AuditLoggingModule } from './auditLogging/auditLogging.module';
import { LoggingService } from './logging.service';

@Module({
  imports: [AuditLoggingModule],
  providers: [LoggingService],
  exports: [LoggingService],
})
@Global()
export class LoggingModule {}
