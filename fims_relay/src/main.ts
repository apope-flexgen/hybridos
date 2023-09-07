import { NestFactory } from '@nestjs/core';
import { DocumentBuilder, SwaggerModule } from '@nestjs/swagger';
import { AppModule } from './app.module';
import { RelayService } from './relay/relay.service';

async function bootstrap() {
  const app = await NestFactory.create(AppModule);
  const relay = app.get<RelayService>(RelayService);
  const FIMS_RELAY_HTTP_PORT = 9150

  const config = new DocumentBuilder()
    .setTitle('fims_relay API Documentation.')
    .setVersion('1.0.0')
    .addTag('fims_relay')
    .build();
  const document = SwaggerModule.createDocument(app, config);
  SwaggerModule.setup('api', app, document);
  
  relay.start();

  app.listen(FIMS_RELAY_HTTP_PORT);
}
bootstrap();
