import { INestApplication, ValidationPipe } from '@nestjs/common'
import { Test, TestingModule } from '@nestjs/testing'
import { INVALID_USERNAME_API } from '../../src/users/validators/IsValidUsername'

import request from './../testReqAgent'

import { HttpExceptionFilter } from '../../src/filters/http-expections.filter'
import { UserNotFoundException } from '../../src/users/exceptions/exceptions'
import { IUsersService, USERS_SERVICE } from '../../src/users/interfaces/users.service.interface'
import { UsersController } from '../../src/users/users.controller'
import { createTestApiApplication } from './../../test/testUtils'
import { AUDIT_LOGGING_SERVICE } from 'src/logging/auditLogging/interfaces/auditLogging.service.interface'
import { RolesGuard } from 'src/auth/guards/roles.guard'
import { SITE_ADMINS_SERVICE } from 'src/siteAdmins/interfaces/siteAdmin.service.interface'
import { useContainer } from 'class-validator'
import { ValidPasswordConstraint } from 'src/users/validators/IsValidPassword'
import * as testUtils from '../testUtils'

describe('UsersController PUT (e2e)', () => {
    let app: INestApplication
    let usersService: IUsersService
    const VALID_PASS = 'Aa1!Bb2@'
    const VALID_USERNAME = 'flexgen_user'
    const VALID_ROLE = 'admin'
    const VALID_ID = '631118fdec0291252e0fba15'

    beforeEach(async () => {
        const moduleFixture: TestingModule = await Test.createTestingModule({
            controllers: [UsersController],
            providers: [
                {
                    provide: USERS_SERVICE,
                    useValue: {
                        update: jest.fn(),
                    },
                },
                ValidPasswordConstraint,
                {
                    provide: SITE_ADMINS_SERVICE,
                    useValue: {
                        find: jest.fn().mockReturnValue(testUtils.site(false, false, false))
                    }
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
        useContainer(moduleFixture, { fallbackOnErrors: true })
        app.useGlobalPipes(new ValidationPipe())
        app.useGlobalFilters(new HttpExceptionFilter())
        usersService = moduleFixture.get(USERS_SERVICE)
        await app.init()
    })

    it('/users/:id (PUT): valid data', () => {
        jest.spyOn(usersService, 'update').mockImplementation(() =>
            Promise.resolve({ id: 'a uuid' })
        )
        const requestBody = {
            role: VALID_ROLE,
            password: VALID_PASS,
            username: VALID_USERNAME,
        }
        const url = `/users/${VALID_ID}`

        return request(app.getHttpServer()).put(url).send(requestBody).expect(200).expect({
            id: 'a uuid',
        })
    })

    it('/users/:id (PUT): invalid user id', () => {
        jest.spyOn(usersService, 'update').mockImplementation(() =>
            Promise.resolve({ id: 'a uuid' })
        )
        const INVALID_ID = '123'
        const requestBody = {
            role: VALID_ROLE,
            password: VALID_PASS,
            username: VALID_USERNAME,
        }
        const response = {
            statusCode: 400,
            message: ['id must be a mongodb id'],
            error: 'Bad Request',
        }
        const url = `/users/${INVALID_ID}`

        return request(app.getHttpServer()).put(url).send(requestBody).expect(400).expect(response)
    })

    it('/users/:id (PUT): invalid username', () => {
        jest.spyOn(usersService, 'update').mockImplementation(() =>
            Promise.resolve({ id: 'a uuid' })
        )
        const INVALID_USERNAME = ''
        const requestBody = {
            role: VALID_ROLE,
            password: VALID_PASS,
            username: INVALID_USERNAME,
        }
        const response = {
            statusCode: 400,
            message: [INVALID_USERNAME_API],
            error: 'Bad Request',
        }
        const url = `/users/${VALID_ID}`

        return request(app.getHttpServer()).put(url).send(requestBody).expect(400).expect(response)
    })

    it('/users/:id (PUT): user not found', () => {
        jest.spyOn(usersService, 'update').mockImplementation(() => {
            throw new UserNotFoundException()
        })
        const requestBody = {
            role: VALID_ROLE,
            password: VALID_PASS,
            username: VALID_USERNAME,
        }
        const response = {
            statusCode: 404,
            message: 'User Not Found',
        }
        const url = `/users/${VALID_ID}`

        return request(app.getHttpServer()).put(url).send(requestBody).expect(404).expect(response)
    })

    it('/users/:id (PUT): unknown exception', () => {
        jest.spyOn(usersService, 'update').mockImplementation(() => {
            throw new Error('Something went wrong.')
        })
        const requestBody = {
            role: VALID_ROLE,
            password: VALID_PASS,
            username: VALID_USERNAME,
        }
        const url = `/users/${VALID_ID}`

        return request(app.getHttpServer()).put(url).send(requestBody).expect(500)
    })

    afterAll(async () => {
        await app.close()
    })
})
