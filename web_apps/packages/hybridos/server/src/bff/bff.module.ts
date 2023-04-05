import { Module } from '@nestjs/common'
import { AssetsPageModule } from './ConfigurablePages/AssetsPage/assetsPage.module'
import { DashboardModule } from './ConfigurablePages/Dashboard/dashboard.module'
import { SchedulerModule } from './SchedulerPage/scheduler.module'
import { EventsModule } from './Events/events.module'
import { SiteStatusModule } from './SiteStatusBar/sitestatus.module'
import { VariableOverrideModule } from './ErcotOverride/ercotOverride.module'

@Module({
    imports: [AssetsPageModule, SchedulerModule, EventsModule, DashboardModule, SiteStatusModule, VariableOverrideModule],
    controllers: [],
    providers: [],
    exports: [],
})
export class BffModule {}
