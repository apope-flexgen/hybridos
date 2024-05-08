import { INestApplication } from '@nestjs/common';
import { WsAdapter } from '@nestjs/platform-ws';
import { Test, TestingModule } from '@nestjs/testing';
import cookieParser from 'cookie-parser';
import { MongoMemoryServer } from 'mongodb-memory-server';

import { AppModule } from '../../src/app.module';
import { AppEnvService } from '../../src/environment/appEnv.service';
import * as testUtils from '../testUtils';
import { PermissionsService } from 'src/permissions/permissions.service';
import { NestSwagger } from 'src/openapi/nestswagger';

describe('Users (Integration)', () => {
  let app: INestApplication;
  let db: MongoMemoryServer;
  let appEnvService: AppEnvService;

  beforeEach(async () => {
    const mongoServer = await MongoMemoryServer.create();
    const moduleFixture: TestingModule = await Test.createTestingModule({
      imports: [AppModule],
    })
      .overrideProvider(AppEnvService)
      .useValue(testUtils.mockAppEnvService(mongoServer.getUri()))
      .overrideProvider(PermissionsService)
      .useValue({ webServerConfigDirectoryPath: () => '' })
      .compile();

    db = mongoServer;

    app = testUtils.createTestApiApplication(moduleFixture);
    appEnvService = moduleFixture.get<AppEnvService>(AppEnvService);
    app.useWebSocketAdapter(new WsAdapter(app));
    await app.init();
  });

  afterAll(() => {
    FIMS.closeConnection();
  });

  it('Generate OpenAPI Documentation', async () => {
    const nestSwagger = new NestSwagger(app);
    nestSwagger.generateDocumentation();
  });

  afterEach(async () => {
    await app.close();

    if (db) {
      await db.stop();
    }
  });
});
