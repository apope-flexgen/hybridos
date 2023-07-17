import { Module } from '@nestjs/common';
import { ConfigModule } from 'src/config/config.module';
import { FimsModule } from '../../fims/fims.module';
import { DBIModule } from 'src/dbi/dbi.module';
import { SiteStatusController } from './sitestatus.controller';
import { SiteStatusGateway } from './sitestatus.gateway';
import { SiteStatusService } from './sitestatus.service';

@Module({
  imports: [FimsModule, ConfigModule, DBIModule],
  controllers: [SiteStatusController],
  providers: [SiteStatusService, SiteStatusGateway],
})
export class SiteStatusModule { }
