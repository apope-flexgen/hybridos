import { INestApplication } from '@nestjs/common';
import { WsAdapter } from '@nestjs/platform-ws';
import { Test, TestingModule } from '@nestjs/testing';
// import * as cookieParser from 'cookie-parser';
import cookieParser from 'cookie-parser';
import { MongoMemoryServer } from 'mongodb-memory-server';

import request from './../../../testReqAgent';

import { AppModule } from '../../../../src/app.module';
import { AppEnvService } from '../../../../src/environment/appEnv.service';
import * as testUtils from '../../../testUtils';
import { PermissionsService } from 'src/permissions/permissions.service';

describe('Role Access - Allowed (Integration)', () => {
  let app: INestApplication;
  let db: MongoMemoryServer;
  let appEnvService: AppEnvService;
  let ADMIN_ACCESS_TOKEN: string;
  let USER_ACCESS_TOKEN: string;

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
    app.use(cookieParser());
    await app.init();
  });

  beforeEach(async () => {
    ADMIN_ACCESS_TOKEN = await testUtils.generateAdminAccessToken(app, db);
  });

  describe('allowed to access protected routes', () => {
    describe('siteAdmins routes', () => {
      const URL = '/site-admins';

      it('GET /site-admins', async () => {
        return await request(app.getHttpServer())
          .get(URL)
          .set('Authorization', ADMIN_ACCESS_TOKEN)
          .then((res) => {
            expect(res.status).toBe(200);
          });
      });

      it('POST /site-admins', async () => {
        const requestBody = testUtils.site(false, false, false);

        return await request(app.getHttpServer())
          .post(URL)
          .set('Authorization', ADMIN_ACCESS_TOKEN)
          .send(requestBody)
          .then((res) => {
            expect(res.status).toBe(201);
          });
      });

      it('POST /site-admins/radius-test', async () => {
        const requestBody = {
          ipAddress: '127.0.0.1',
          port: '8080',
          secret_phrase: 'secret',
          wait_time: 5000,
          username: testUtils.VALID_USERNAME,
          password: testUtils.VALID_PASS,
        };

        return await request(app.getHttpServer())
          .post(`${URL}/radius-test`)
          .set('Authorization', ADMIN_ACCESS_TOKEN)
          .send(requestBody)
          .then((res) => {
            expect(res.status).not.toBe(403);
          });
      });
    });

    describe('users routes', () => {
      const URL = '/users';

      it('GET /users can be accessed by an admin', async () => {
        return await request(app.getHttpServer())
          .get(URL)
          .set('Authorization', ADMIN_ACCESS_TOKEN)
          .then((res) => {
            expect(res.status).toBe(200);
          });
      });

      it('GET /users can be accessed by a user', async () => {
        USER_ACCESS_TOKEN = await testUtils.generateUserAccessToken(app, db);
        return await request(app.getHttpServer())
          .get(URL)
          .set('Authorization', USER_ACCESS_TOKEN)
          .then((res) => {
            expect(res.status).toBe(200);
          });
      });
    });
  });

  afterEach(async () => {
    await testUtils.dropDatabase(db.getUri());
  });

  afterAll(async () => {
    FIMS.closeConnection();
    await app.close();

    if (db) {
      await db.stop();
    }
  });
});
