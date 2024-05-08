import { Module } from '@nestjs/common';
import { FimsModule } from '../../fims/fims.module';
import { SystemStatusController } from './systemStatus.controller';
import { SystemStatusService } from './systemStatus.service';
import { SystemStatusGateway } from './systemStatus.gateway';
import { AuthModule } from '../../auth/auth.module';

@Module({
  imports: [FimsModule, AuthModule],
  controllers: [SystemStatusController],
  providers: [SystemStatusService, SystemStatusGateway],
})
export class SystemStatusModule {}
