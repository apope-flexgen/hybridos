import { Module } from '@nestjs/common';
import { WebUIConfigService } from './webUIConfig.service';
import { UI_CONFIG_SERVICE } from './webUIConfig.service.interface';
import { AppEnvModule } from '../environment/appEnv.module';
import { WebUIConfigController } from './webUIConfig.controller';

@Module({
  imports: [AppEnvModule],
  controllers: [WebUIConfigController],
  providers: [
    {
      provide: UI_CONFIG_SERVICE,
      useClass: WebUIConfigService,
    },
  ],
})
export class WebUIConfigModule {}
