import { INestApplication } from '@nestjs/common';
import { JwtService } from '@nestjs/jwt';
import { WsAdapter } from '@nestjs/platform-ws';
import { Test, TestingModule } from '@nestjs/testing';
// import * as cookieParser from 'cookie-parser';
import cookieParser from 'cookie-parser';
import { ObjectId } from 'mongodb';
import { MongoMemoryServer } from 'mongodb-memory-server';

import request from './../../../testReqAgent';

import { AppModule } from '../../../../src/app.module';
import { AppEnvService } from '../../../../src/environment/appEnv.service';
import * as testUtils from '../../../testUtils';
import { ValidPasswordConstraint } from 'src/users/validators/IsValidPassword';
import { SITE_ADMINS_SERVICE } from 'src/siteAdmins/interfaces/siteAdmin.service.interface';
import { useContainer } from 'class-validator';
import { PermissionsService } from 'src/permissions/permissions.service';

const OLD_PASS = testUtils.VALID_PASS;
const INVALID_PASS = 'abc';

let jwtService: JwtService;
let VALID_ACCESS_TOKEN: string;
let INVALID_ACCESS_TOKEN: string;

const initializeAccessToken = async (userID: ObjectId) => {
  const payload = {
    username: testUtils.VALID_USERNAME,
    oneTime: 'password-expiration',
    sub: userID,
  };
  VALID_ACCESS_TOKEN = `Bearer ${jwtService.sign(payload, {
    expiresIn: '12d',
    secret: 'supersecretkey-oneTimeUse-password-expiration',
  })}`;
};

describe('Authentication (Integration)', () => {
  let app: INestApplication;
  let db: MongoMemoryServer;

  beforeEach(async () => {
    const mongoServer = await MongoMemoryServer.create();

    const moduleFixture: TestingModule = await Test.createTestingModule({
      imports: [AppModule],
      providers: [
        ValidPasswordConstraint,
        {
          provide: SITE_ADMINS_SERVICE,
          useValue: {
            find: jest.fn(),
          },
        },
      ],
    })
      .overrideProvider(AppEnvService)
      .useValue(testUtils.mockAppEnvService(mongoServer.getUri()))
      .overrideProvider(PermissionsService)
      .useValue({ webServerConfigDirectoryPath: () => '' })
      .compile();

    db = mongoServer;

    app = testUtils.createTestApiApplication(moduleFixture);
    useContainer(moduleFixture, { fallbackOnErrors: true });
    app.useWebSocketAdapter(new WsAdapter(app));
    app.use(cookieParser());

    jwtService = moduleFixture.get<JwtService>(JwtService);
    INVALID_ACCESS_TOKEN = `Bearer ${jwtService.sign(
      {},
      {
        secret: 'incorrectsignature',
      },
    )}`;

    await app.init();
  });

  describe('/login/passExp responses', () => {
    const URL = '/login/passExp';

    describe('mfa is not enabled', () => {
      beforeEach(async () => {
        await testUtils.initializeSite(db.getUri(), testUtils.site(false, true, false));
        await testUtils
          .initializeUser(db.getUri(), await testUtils.adminUser(true, false))
          .then(async (userID) => {
            await initializeAccessToken(userID);
          });
      });

      it('successfully logs in user', () => {
        const requestBody = {
          username: testUtils.VALID_USERNAME,
          updatedPassword: testUtils.NEW_PASS,
        };

        return request(app.getHttpServer())
          .post(URL)
          .set('Authorization', VALID_ACCESS_TOKEN)
          .send(requestBody)
          .then((res) => {
            expect(res.status).toBe(200);
            expect(res.body).toEqual(expect.objectContaining(testUtils.SUCCESS_RESPONSE));
            expect(res.headers['set-cookie']).toBeTruthy();
          });
      });

      it('fails (password was not updated)', () => {
        const requestBody = {
          username: testUtils.VALID_USERNAME,
          updatedPassword: OLD_PASS,
        };

        return request(app.getHttpServer())
          .post(URL)
          .set('Authorization', VALID_ACCESS_TOKEN)
          .send(requestBody)
          .then((res) => {
            expect(res.status).toBe(400);
            expect(res.body).toStrictEqual({
              statusCode: 400,
              message: 'New password must not match current password',
            });
            expect(res.headers['set-cookie']).toBeFalsy();
          });
      });

      it('fails (invalid password)', () => {
        const requestBody = {
          username: testUtils.VALID_USERNAME,
          updatedPassword: INVALID_PASS,
        };

        return request(app.getHttpServer())
          .post(URL)
          .set('Authorization', VALID_ACCESS_TOKEN)
          .send(requestBody)
          .then((res) => {
            expect(res.status).toBe(400);
            expect(res.body).toStrictEqual({
              error: 'Bad Request',
              message: ['password is invalid'],
              statusCode: 400,
            });
            expect(res.headers['set-cookie']).toBeFalsy();
          });
      });

      it('fails (invalid accessToken)', () => {
        const requestBody = {
          username: testUtils.VALID_USERNAME,
          updatedPassword: testUtils.NEW_PASS,
        };

        return request(app.getHttpServer())
          .post(URL)
          .set('Authorization', INVALID_ACCESS_TOKEN)
          .send(requestBody)
          .then((res) => {
            expect(res.status).toBe(401);
            expect(res.body).toStrictEqual({
              statusCode: 401,
              message: 'Unauthorized',
            });
            expect(res.headers['set-cookie']).toBeFalsy();
          });
      });
    });

    describe('mfa is enabled', () => {
      beforeEach(async () => {
        await testUtils.initializeSite(db.getUri(), testUtils.site(true, true, false));
      });

      it('successfully returns mfa response (enabled)', async () => {
        await testUtils
          .initializeUser(db.getUri(), await testUtils.adminUser(false, true))
          .then((userID) => {
            initializeAccessToken(userID);
          });

        const requestBody = {
          username: testUtils.VALID_USERNAME,
          updatedPassword: testUtils.NEW_PASS,
        };

        await request(app.getHttpServer())
          .post(URL)
          .set('Authorization', VALID_ACCESS_TOKEN)
          .send(requestBody)
          .then((res) => {
            expect(res.status).toBe(200);
            expect(res.body.username).toBe(testUtils.VALID_USERNAME);
            expect(res.body).not.toHaveProperty('qrCode');
            expect(res.body.accessToken).toBeTruthy();
          });
        return;
      });

      it('successfully returns mfa response (not yet enabled)', async () => {
        await testUtils
          .initializeUser(db.getUri(), await testUtils.adminUser(false, false))
          .then((userID) => {
            initializeAccessToken(userID);
          });

        const requestBody = {
          username: testUtils.VALID_USERNAME,
          updatedPassword: testUtils.NEW_PASS,
        };

        return await request(app.getHttpServer())
          .post(URL)
          .set('Authorization', VALID_ACCESS_TOKEN)
          .send(requestBody)
          .then((res) => {
            expect(res.status).toBe(200);
            expect(res.body.username).toBe(testUtils.VALID_USERNAME);
            expect(res.body).toHaveProperty('qrCode');
            expect(res.body.accessToken).toBeTruthy();
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
