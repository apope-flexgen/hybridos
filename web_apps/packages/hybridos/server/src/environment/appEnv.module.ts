import { Global, Module } from '@nestjs/common';
import { ConfigModule } from 'src/config/config.module';
import { AppEnvService } from './appEnv.service';
import { APP_ENV_SERVICE } from 'src/environment/appEnv.interface';

@Global()
@Module({
  imports: [ConfigModule],
  providers: [{ provide: APP_ENV_SERVICE, useClass: AppEnvService }],
  exports: [APP_ENV_SERVICE],
})
export class AppEnvModule {}
