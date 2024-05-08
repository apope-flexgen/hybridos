import { INestApplication, ValidationPipe } from '@nestjs/common';
import { getModelToken } from '@nestjs/mongoose';
import { Test, TestingModule } from '@nestjs/testing';

import request from './../testReqAgent';
import { createTestApiApplication } from '../../test/testUtils';

import { SiteAdminsService } from '../../src/siteAdmins/siteAdmins.service';
import { HttpExceptionFilter } from '../../src/filters/http-expections.filter';
import { UserNotFoundException } from '../../src/users/exceptions/exceptions';
import { IUsersService, USERS_SERVICE } from '../../src/users/interfaces/users.service.interface';
import { User } from '../../src/users/user.schema';
import { UsersController } from '../../src/users/users.controller';
import { AUDIT_LOGGING_SERVICE } from 'src/logging/auditLogging/interfaces/auditLogging.service.interface';
import { RolesGuard } from 'src/auth/guards/roles.guard';

describe('UsersController DELETE (e2e)', () => {
  let app: INestApplication;
  let usersService: IUsersService;
  const TEST_ID = '631118fdec0291252e0fba15';

  beforeEach(async () => {
    const moduleFixture: TestingModule = await Test.createTestingModule({
      controllers: [UsersController],
      providers: [
        {
          provide: USERS_SERVICE,
          useValue: {
            delete: jest.fn(),
          },
        },
        {
          provide: getModelToken(User.name),
          useValue: {
            deleteOne: jest.fn(),
          },
        },
        SiteAdminsService,
        {
          provide: getModelToken('siteadmins'),
          useValue: {
            findOne: jest.fn(),
            create: jest.fn(),
          },
        },
        {
          provide: AUDIT_LOGGING_SERVICE,
          useValue: {
            postAuditLog: jest.fn(),
          },
        },
      ],
    })
      .overrideGuard(RolesGuard)
      .useValue({ canActivate: () => true })
      .compile();

    app = createTestApiApplication(moduleFixture);
    app.useGlobalPipes(new ValidationPipe());
    app.useGlobalFilters(new HttpExceptionFilter());
    usersService = moduleFixture.get(USERS_SERVICE);
    await app.init();
  });

  it('/users/:id (DELETE): successfully deletes user', () => {
    const URL = `/users/${TEST_ID}`;
    jest
      .spyOn(usersService, 'delete')
      .mockImplementation(() => Promise.resolve({ userDeleted: true }));
    return request(app.getHttpServer()).del(URL).expect(200);
  });

  it('/users/:id (DELETE): bad url', () => {
    const URL = `/users/delete/${TEST_ID}`;
    return request(app.getHttpServer()).del(URL).expect(404);
  });

  it('/users/:id (DELETE): method not allowed, returns 404', () => {
    // NOTE: Nest returns a 404 instead of a 405 when a method is not allowed.
    const URL = `/users/${TEST_ID}`;
    return request(app.getHttpServer()).post(URL).expect(404);
  });

  it('/users/:id (DELETE): failed to delete user', () => {
    const URL = `/users/${TEST_ID}`;
    jest.spyOn(usersService, 'delete').mockImplementation(() => {
      throw new Error('Unknown error!');
    });
    return request(app.getHttpServer()).del(URL).expect(500);
  });

  it('/users/:id (DELETE): user not found', () => {
    const URL = `/users/${TEST_ID}`;
    const responseBody = {
      statusCode: 404,
      message: 'User Not Found',
    };
    jest.spyOn(usersService, 'delete').mockImplementation(() => {
      throw new UserNotFoundException();
    });
    return request(app.getHttpServer()).del(URL).expect(404).expect(responseBody);
  });

  it('/users/:id (DELETE): invalid id', () => {
    const URL = '/users/123';
    const responseBody = {
      statusCode: 400,
      message: ['id must be a mongodb id'],
      error: 'Bad Request',
    };
    jest.spyOn(usersService, 'delete').mockImplementation(() => {
      throw new UserNotFoundException();
    });
    return request(app.getHttpServer()).del(URL).expect(400).expect(responseBody);
  });

  afterAll(async () => {
    await app.close();
  });
});
