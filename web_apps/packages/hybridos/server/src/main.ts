declare const module: any;

import { NestFactory } from '@nestjs/core';
import { AuthWsAdapter } from './adapters/authWs.adapter';
// import * as cookieParser from 'cookie-parser';
import cookieParser from 'cookie-parser';

import { AppModule } from './app.module';
import { AppEnvService } from './environment/appEnv.service';
import { NestSwagger } from './openapi/nestswagger';
import { RequestMethod } from '@nestjs/common';
import { useContainer } from 'class-validator';
import { ProcessArgvValidation } from './startup/ProcessArgValidation';
import { ConfigModule } from './config/config.module';

// eslint-disable-next-line max-statements
async function bootstrap() {
  require('events').EventEmitter.defaultMaxListeners = 25;
  const processArgs = new ProcessArgvValidation();
  const httpsOptions = {
    key: processArgs.ssl.keyFile,
    cert: processArgs.ssl.certFile,
  };

  ConfigModule.setWebUiConfigPath(processArgs.webUIConfigPath);
  ConfigModule.setWebServerConfigPath(processArgs.webServerConfigPath);
  ConfigModule.setWebUiBuildPath(processArgs.webUIBuildPath);

  const app = await NestFactory.create(AppModule, {
    httpsOptions,
  });
  useContainer(app.select(AppModule), { fallbackOnErrors: true });

  const appEnvService = app.get(AppEnvService);
  const DEV_MODE_PORT = 3001;
  const PORT = processArgs.isDev ? DEV_MODE_PORT : appEnvService.getAppServerPort();

  app.setGlobalPrefix('api', {
    exclude: [{ path: 'rest/:endpoint*', method: RequestMethod.ALL }],
  });
  // Generate OpenAPI Documentation
  if (process.env.NODE_ENV === 'dev' || process.env.NODE_ENV === 'test') {
    const nestSwagger = new NestSwagger(app);
    nestSwagger.generateDocumentation();
  }

  app.use(cookieParser());
  app.useWebSocketAdapter(new AuthWsAdapter(app));
  app.enableCors({
    origin: ['https://172.16.1.80', 'http://localhost'],
    credentials: true,
  });
  await app.listen(PORT);
  console.log('web_server listening on port ' + PORT);

  if (module.hot) {
    module.hot.accept();
    module.hot.dispose(() => app.close());
  }
}
bootstrap();
