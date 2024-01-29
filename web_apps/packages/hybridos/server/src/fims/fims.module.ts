import { Module } from '@nestjs/common';
import { JwtModule } from '@nestjs/jwt';
import { ThrottlerModule } from '@nestjs/throttler';
import { PermissionsModule } from '../permissions/permissions.module';
import { FimsController } from './fims.controller';
import { FimsGateway } from './fims.gateway';
import { FimsService } from './fims.service';
import { FIMS_SERVICE } from './interfaces/fims.interface';
import { SOCKET_AUTH_SERVICE } from './interfaces/socketAuth.service.interface';
import { SocketAuthService } from './socketAuth.service';
import { APP_ENV_SERVICE, IAppEnvService } from 'src/environment/appEnv.interface';

@Module({
  imports: [
    PermissionsModule,
    JwtModule.registerAsync({
      useFactory: async (appEnv: IAppEnvService) => ({
        secret: appEnv.getAccessTokenSecretFimsSocket(),
      }),
      inject: [APP_ENV_SERVICE],
    }),
    ThrottlerModule.forRoot(),
  ],
  controllers: [FimsController],
  providers: [
    {
      provide: FIMS_SERVICE,
      useClass: FimsService,
    },
    {
      provide: SOCKET_AUTH_SERVICE,
      useClass: SocketAuthService,
    },
    FimsGateway,
  ],
  exports: [FIMS_SERVICE],
})
export class FimsModule {}
