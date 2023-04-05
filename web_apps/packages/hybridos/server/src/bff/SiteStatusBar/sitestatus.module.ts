import { Module } from '@nestjs/common';
import { FimsModule } from '../../fims/fims.module';
import { SiteStatusController } from './sitestatus.controller';
import { SiteStatusGateway } from './sitestatus.gateway';
import { SiteStatusService } from './sitestatus.service';

@Module({
  imports: [FimsModule],
  controllers: [SiteStatusController],
  providers: [SiteStatusService, SiteStatusGateway],
})
export class SiteStatusModule {}
