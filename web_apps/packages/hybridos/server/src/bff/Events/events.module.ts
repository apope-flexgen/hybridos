import { Module } from '@nestjs/common';
import { FimsModule } from '../../fims/fims.module';
import { EventsController } from './events.controller';
import { EventsService } from './events.service';

@Module({
  imports: [FimsModule],
  controllers: [EventsController],
  providers: [EventsService],
})
export class EventsModule {}
