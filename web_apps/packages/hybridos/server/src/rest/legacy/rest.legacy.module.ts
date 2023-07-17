import { Module } from '@nestjs/common';
import { AppEnvModule } from 'src/environment/appEnv.module';
import { FimsModule } from 'src/fims/fims.module';
import { PermissionsModule } from 'src/permissions/permissions.module';
import { LegacyRestController } from './rest.legacy.controller';
import { LegacyRestService } from './rest.legacy.service';
import { AggregatedEndpointsService } from './aggregatedEndpoints.service';
import { LEGACY_REST_SERVICE } from './interfaces/rest.legacy.interface';

@Module({
  imports: [FimsModule, AppEnvModule, PermissionsModule],
  controllers: [LegacyRestController],
  providers: [
    AggregatedEndpointsService,
    { provide: LEGACY_REST_SERVICE, useClass: LegacyRestService },
  ],
  exports: [LEGACY_REST_SERVICE],
})
export class LegacyRestModule {}
