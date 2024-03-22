import { Module } from '@nestjs/common'
import { FimsModule } from '../../fims/fims.module'
import { AlertsController } from './alerts.controller'
import { AlertsService } from './alerts.service'

@Module({
    imports: [FimsModule],
    controllers: [AlertsController],
    providers: [AlertsService],
})
export class AlertsModule {}
