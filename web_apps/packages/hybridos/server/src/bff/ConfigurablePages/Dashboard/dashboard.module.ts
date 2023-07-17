import { Module } from '@nestjs/common';
import { DashboardGateway } from './dashboard.gateway';
import { DashboardService } from './dashboard.service';
import { FimsModule } from '../../../fims/fims.module';
import { DBIModule } from 'src/dbi/dbi.module';
import { DashboardLayoutInterceptor } from './tableDashboard.interceptor';

@Module({
  imports: [FimsModule, DBIModule],
  controllers: [],
  providers: [DashboardService, DashboardGateway, DashboardLayoutInterceptor],
})
export class DashboardModule {}
