import { INestApplication } from '@nestjs/common';
import { WsAdapter } from '@nestjs/platform-ws';
import { Test, TestingModule } from '@nestjs/testing';
import { isMongoId, useContainer } from 'class-validator';
// import * as cookieParser from 'cookie-parser';
import cookieParser from 'cookie-parser';
import { MongoMemoryServer } from 'mongodb-memory-server';

import request from './../testReqAgent';
import { Roles } from '../../../shared/types/api/Users/Users.types';

import { AppModule } from '../../src/app.module';
import { AppEnvService } from '../../src/environment/appEnv.service';
import * as testUtils from '../testUtils';
import { WsThrottlerGuard } from 'src/fims/wsthrottler.guard';
import { ThrottlerGuard } from '@nestjs/throttler';
import { ValidPasswordConstraint } from 'src/users/validators/IsValidPassword';
import { SITE_ADMINS_SERVICE } from 'src/siteAdmins/interfaces/siteAdmin.service.interface';
import { PermissionsService } from 'src/permissions/permissions.service';

describe('Users (Integration)', () => {
  let app: INestApplication;
  let db: MongoMemoryServer;
  let appEnvService: AppEnvService;
  let VALID_ACCESS_TOKEN: string;

  const VALID_USER_DATA = {
    role: Roles.Admin,
    password: 'Aa1!Bb2@',
    username: 'flexgen_user',
  };

  beforeEach(async () => {
    const mongoServer = await MongoMemoryServer.create();

    const moduleFixture: TestingModule = await Test.createTestingModule({
      imports: [AppModule],
      providers: [
        ValidPasswordConstraint,
        {
          provide: SITE_ADMINS_SERVICE,
          useValue: {
            find: jest.fn().mockReturnValue(testUtils.site(false, false, false)),
          },
        },
      ],
    })
      .overrideProvider(AppEnvService)
      .useValue(testUtils.mockAppEnvService(mongoServer.getUri()))
      .overrideProvider(PermissionsService)
      .useValue({ webServerConfigDirectoryPath: () => '' })
      .overrideGuard(ThrottlerGuard)
      .useValue({ handleRequest: () => true })
      .compile();

    db = mongoServer;

    app = testUtils.createTestApiApplication(moduleFixture);
    useContainer(moduleFixture, { fallbackOnErrors: true });
    appEnvService = moduleFixture.get<AppEnvService>(AppEnvService);
    app.useWebSocketAdapter(new WsAdapter(app));
    app.use(cookieParser());
    await app.init();
  });

  beforeEach(async () => {
    VALID_ACCESS_TOKEN = await testUtils.generateAdminAccessToken(app, db);
  });

  afterAll(() => {
    FIMS.closeConnection();
  });

  // TODO: Fix default user bug
  // it('Should create a User', async () => {
  //   const res = await request(app.getHttpServer())
  //     .post('/users')
  //     .set('Authorization', `${VALID_ACCESS_TOKEN}`)
  //     .send(VALID_USER_DATA);

  //   expect(res.status).toBe(201);
  //   expect(isMongoId(res.body.id)).toBe(true);
  // });

  it('Should read a User', async () => {
    const createData = {
      role: VALID_USER_DATA.role,
      username: VALID_USER_DATA.username,
    };
    const USER_ID = await testUtils.initializeUser(appEnvService.getMongoUri(), createData);
    const URL = `/users/${USER_ID}`;

    const res = await request(app.getHttpServer())
      .get(URL)
      .set('Authorization', `${VALID_ACCESS_TOKEN}`);

    expect(res.status).toBe(200);
    expect(res.body).toEqual(
      expect.objectContaining({
        id: USER_ID.toString(),
        role: VALID_USER_DATA.role,
        username: VALID_USER_DATA.username,
      }),
    );
  });

  it('Should read all by role', async () => {
    const createData = {
      role: VALID_USER_DATA.role,
      username: VALID_USER_DATA.username,
    };

    const USER_ID = await testUtils.initializeUser(appEnvService.getMongoUri(), createData);
    const URL = `/users?role=${createData.role}`;

    const res = await request(app.getHttpServer())
      .get(URL)
      .set('Authorization', `${VALID_ACCESS_TOKEN}`);

    expect(res.status).toBe(200);
    expect(res.body.users.length).toEqual(1);
    expect(res.body.users[0].username).toEqual(VALID_USER_DATA.username);
    expect(res.body.users[0].id).toEqual(USER_ID.toString());
  });

  it('Should update a User', async () => {
    const EDIT_ROLE = 'developer';
    const USER_ID = await testUtils.initializeUser(appEnvService.getMongoUri(), VALID_USER_DATA);
    const URL = `/users/${USER_ID}`;
    const editData = {
      username: VALID_USER_DATA.username,
      password: VALID_USER_DATA.password,
      role: EDIT_ROLE,
    };

    const res = await request(app.getHttpServer())
      .put(URL)
      .set('Authorization', `${VALID_ACCESS_TOKEN}`)
      .send(editData);

    expect(res.status).toBe(200);
    expect(res.body.role).toBe(EDIT_ROLE);
  });

  it('Should delete a User', async () => {
    const USER_ID = await testUtils.initializeUser(appEnvService.getMongoUri(), VALID_USER_DATA);
    const URL = `/users/${USER_ID}`;

    return request(app.getHttpServer())
      .del(URL)
      .set('Authorization', `${VALID_ACCESS_TOKEN}`)
      .expect(200)
      .expect({ userDeleted: true });
  });

  afterEach(async () => {
    await app.close();

    if (db) {
      await db.stop();
    }
  });
});
