import { Module } from '@nestjs/common';
import { ConfigModule } from 'src/config/config.module';
import { FimsModule } from '../../fims/fims.module';
import { DBIModule } from 'src/dbi/dbi.module';
import { SiteStatusController } from './sitestatus.controller';
import { SiteStatusGateway } from './sitestatus.gateway';
import { SiteStatusService } from './sitestatus.service';
import { SITE_STATUS_SERVICE } from 'src/bff/SiteStatusBar/sitestatus.interface';

@Module({
  imports: [FimsModule, ConfigModule, DBIModule],
  controllers: [SiteStatusController],
  providers: [{ provide: SITE_STATUS_SERVICE, useClass: SiteStatusService }, SiteStatusGateway],
})
export class SiteStatusModule {}
