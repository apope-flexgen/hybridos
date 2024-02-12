import { Global, Module } from '@nestjs/common';
import { ConfigModule } from 'src/config/config.module';
import { AppEnvService } from './appEnv.service';
import { APP_ENV_SERVICE } from 'src/environment/appEnv.interface';
import { SecretGenerationModule } from 'src/environment/secretGeneration/secretGeneration.module';

@Global()
@Module({
  imports: [ConfigModule, SecretGenerationModule],
  providers: [{ provide: APP_ENV_SERVICE, useClass: AppEnvService }],
  exports: [APP_ENV_SERVICE],
})
export class AppEnvModule {}
