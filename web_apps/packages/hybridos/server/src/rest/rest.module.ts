import { Module } from '@nestjs/common'
import { AppEnvModule } from '../environment/appEnv.module'
import { FimsModule } from '../fims/fims.module'
import { PermissionsModule } from '../permissions/permissions.module'
import { RestController } from './rest.controller'
import { RestService } from './rest.service'

@Module({
    imports: [FimsModule, PermissionsModule, AppEnvModule],
    controllers: [RestController],
    providers: [RestService],
})
export class RestModule {}
