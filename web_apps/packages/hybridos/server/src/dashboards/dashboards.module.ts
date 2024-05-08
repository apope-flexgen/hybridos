import { Module } from '@nestjs/common';
import { DashboardsController } from './dashboards.controller';
import { DashboardsService } from './dashboards.service';
import { DASHBOARDS_SERVICE } from './dashboards.constants';
import { DBIModule } from 'src/dbi/dbi.module';

@Module({
  imports: [DBIModule],
  controllers: [DashboardsController],
  providers: [
    {
      useClass: DashboardsService,
      provide: DASHBOARDS_SERVICE,
    },
  ],
  exports: [],
})
export class DashboardsModule {}
