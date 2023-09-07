import { Module } from '@nestjs/common';
import { ApiModule } from 'src/api/api.module';
import { RelayService } from './relay.service';

@Module({
  imports: [ApiModule],
  providers: [RelayService]
})
export class RelayModule {}
