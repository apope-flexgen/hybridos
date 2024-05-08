import { Module } from '@nestjs/common';
import { SchedulerController } from './scheduler.controller';
import { SchedulerGateway } from './scheduler.gateway';
import { SchedulerService } from './scheduler.service';
import { FimsModule } from '../../fims/fims.module';
import { AuthModule } from '../../auth/auth.module';

@Module({
  imports: [FimsModule, AuthModule],
  controllers: [SchedulerController],
  providers: [SchedulerService, SchedulerGateway],
})
export class SchedulerModule {}
