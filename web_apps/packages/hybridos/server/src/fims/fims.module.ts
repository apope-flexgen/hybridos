import { Module } from '@nestjs/common'
import { JwtModule } from '@nestjs/jwt'
import { AppEnvService } from '../environment/appEnv.service'
import { PermissionsModule } from '../permissions/permissions.module'
import { FimsController } from './fims.controller'
import { FimsGateway } from './fims.gateway'
import { FimsService } from './fims.service'
import { FIMS_SERVICE } from './interfaces/fims.interface'
import { SOCKET_AUTH_SERVICE } from './interfaces/socketAuth.service.interface'
import { SocketAuthService } from './socketAuth.service'

@Module({
    imports: [
        PermissionsModule,
        JwtModule.registerAsync({
            useFactory: async (appEnv: AppEnvService) => ({
                secret: appEnv.getAccessTokenSecretFimsSocket(),
            }),
            inject: [AppEnvService],
        }),
    ],
    controllers: [FimsController],
    providers: [
        { provide: FIMS_SERVICE, useClass: FimsService },
        {
            provide: SOCKET_AUTH_SERVICE,
            useClass: SocketAuthService,
        },
        FimsGateway,
    ],
    exports: [
        {
            useClass: FimsService,
            provide: FIMS_SERVICE,
        },
    ],
})
export class FimsModule {}
