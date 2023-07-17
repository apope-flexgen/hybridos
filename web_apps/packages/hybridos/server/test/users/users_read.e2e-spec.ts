// TODO: fix this rule
/* eslint-disable max-nested-callbacks */
import { INestApplication, ValidationPipe } from '@nestjs/common'
import { Test, TestingModule } from '@nestjs/testing'
import request from './../testReqAgent'
import { createTestApiApplication } from '../../test/testUtils'

import { HttpExceptionFilter } from '../../src/filters/http-expections.filter'
import { UserNotFoundException } from '../../src/users/exceptions/exceptions'
import { IUsersService, USERS_SERVICE } from '../../src/users/interfaces/users.service.interface'
import { UsersController } from '../../src/users/users.controller'
import { AUDIT_LOGGING_SERVICE } from 'src/logging/auditLogging/interfaces/auditLogging.service.interface'
import { RolesGuard } from 'src/auth/guards/roles.guard'

describe('UsersController READ (e2e)', () => {
    let app: INestApplication
    let usersService: IUsersService
    const TEST_ID = '631118fdec0291252e0fba15'
    const TEST_ROLE = 'admin'

    beforeEach(async () => {
        const moduleFixture: TestingModule = await Test.createTestingModule({
            controllers: [UsersController],
            providers: [
                {
                    provide: USERS_SERVICE,
                    useValue: {
                        readById: jest.fn(),
                        all: jest.fn(),
                    },
                },
                {
                    provide: AUDIT_LOGGING_SERVICE,
                    useValue: {
                        postAuditLog: jest.fn()
                    },
                },
            ],
        }).overrideGuard(RolesGuard).useValue({ canActivate: () => true }).compile();

        app = createTestApiApplication(moduleFixture)
        app.useGlobalPipes(new ValidationPipe())
        app.useGlobalFilters(new HttpExceptionFilter())
        usersService = app.get(USERS_SERVICE)
        await app.init()
    })

    describe('read user by id', () => {
        it('/users/:id (READ): successfully reads user', () => {
            const URL = `/users/${TEST_ID}`
            jest.spyOn(usersService, 'readById').mockImplementation(() =>
                Promise.resolve({ id: 'user_id' })
            )
            return request(app.getHttpServer()).get(URL).expect(200)
        })

        it('/users/:id (READ): method not allowed, returns 404', () => {
            const URL = `/users/${TEST_ID}`
            return request(app.getHttpServer()).copy(URL).expect(404)
        })

        it('/users/:id (READ): user not found', () => {
            const URL = `/users/${TEST_ID}`
            const responseBody = {
                statusCode: 404,
                message: 'User Not Found',
            }
            jest.spyOn(usersService, 'readById').mockImplementation(() => {
                throw new UserNotFoundException()
            })
            return request(app.getHttpServer()).get(URL).expect(404).expect(responseBody)
        })

        it('/users/:id (READ): failed to read user, unknown exception.', () => {
            const URL = `/users/${TEST_ID}`
            jest.spyOn(usersService, 'readById').mockImplementation(() => {
                throw new Error('Some random error!')
            })
            return request(app.getHttpServer()).get(URL).expect(500)
        })

        it('/users/:id (READ): invalid id', () => {
            const URL = '/users/123'
            const responseBody = {
                statusCode: 400,
                message: ['id must be a mongodb id'],
                error: 'Bad Request',
            }
            jest.spyOn(usersService, 'readById').mockImplementation(() => {
                throw new UserNotFoundException()
            })
            return request(app.getHttpServer()).del(URL).expect(400).expect(responseBody)
        })
    })

    describe('read all users', () => {
        it('/users (ALL): invalid query', async () => {
            const URL = '/users?role=bad_admin'
            const responseBody = {
                statusCode: 400,
                message: [
                    'role is invalid, valid roles are [user,admin,rest,developer,observer,restreadwrite]',
                ],
                error: 'Bad Request',
            }
            jest.spyOn(usersService, 'all').mockImplementation(() => Promise.resolve(null))
            return request(app.getHttpServer()).get(URL).expect(400).expect(responseBody)
        })

        it('/users (ALL): unknown error in service', async () => {
            const URL = `/users?role=${TEST_ROLE}`
            jest.spyOn(usersService, 'all').mockImplementation(() => {
                throw new Error('Some random error!')
            })
            return request(app.getHttpServer()).get(URL).expect(500)
        })

        it('/users (ALL): success', async () => {
            const URL = `/users?role=${TEST_ROLE}`
            jest.spyOn(usersService, 'all').mockImplementation(() => Promise.resolve({ users: [] }))
            return request(app.getHttpServer()).get(URL).expect(200)
        })
    })

    afterAll(async () => {
        await app.close()
    })
})
