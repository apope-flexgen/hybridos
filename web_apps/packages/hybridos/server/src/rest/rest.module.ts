import { Module } from '@nestjs/common';
import { AppEnvModule } from 'src/environment/appEnv.module';
import { FimsModule } from 'src/fims/fims.module';
import { PermissionsModule } from 'src/permissions/permissions.module';
import { REST_SERVICE } from './interfaces/rest.interface';
import { LegacyRestModule } from './legacy/rest.legacy.module';
import { RestController } from './rest.controller';
import { RestService } from './rest.service';

@Module({
  imports: [FimsModule, AppEnvModule, PermissionsModule, LegacyRestModule],
  controllers: [RestController],
  providers: [{ provide: REST_SERVICE, useClass: RestService }],
})
export class RestModule {}
