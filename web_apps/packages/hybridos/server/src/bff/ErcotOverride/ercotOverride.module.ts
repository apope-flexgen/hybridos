import { Module } from '@nestjs/common';
import { FimsModule } from '../../fims/fims.module';
import { ErcotOverrideService } from './ercotOverride.service';
import { ErcotOverrideController } from './ercotOverride.controller';
import { ErcotOverrideGateway } from './ercotOverride.gateway';
import { AuthModule } from '../../auth/auth.module';

@Module({
  imports: [FimsModule, AuthModule],
  controllers: [ErcotOverrideController],
  providers: [ErcotOverrideService, ErcotOverrideGateway],
})
export class ErcotOverrideModule {}
