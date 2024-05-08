import { Module } from '@nestjs/common';
import { FimsModule } from '../../fims/fims.module';
import { AlertsController } from './alerts.controller';
import { AlertsService } from './alerts.service';
import { AlertsGateway } from './alerts.gateway';

@Module({
  imports: [FimsModule],
  controllers: [AlertsController],
  providers: [AlertsService, AlertsGateway],
})
export class AlertsModule {}
