import { Module } from '@nestjs/common'
import { AssetsPageModule } from './ConfigurablePages/AssetsPage/assetsPage.module'
import { DashboardModule } from './ConfigurablePages/Dashboard/dashboard.module'
import { SchedulerModule } from './SchedulerPage/scheduler.module'
import { EventsModule } from './Events/events.module'
import { SiteStatusModule } from './SiteStatusBar/sitestatus.module'
import { ErcotOverrideModule } from './ErcotOverride/ercotOverride.module'
import { LayoutsModule } from './Layouts/layouts.module'
import { SystemStatusModule } from './SystemStatus/systemStatus.module'
import { AlertsModule } from './Alerts/alerts.module'

@Module({
    imports: [
        AssetsPageModule,
        SchedulerModule,
        EventsModule,
        AlertsModule,
        DashboardModule,
        SiteStatusModule,
        ErcotOverrideModule,
        LayoutsModule,
        SystemStatusModule,
    ],
    controllers: [],
    providers: [],
    exports: [],
})
export class BffModule {}
