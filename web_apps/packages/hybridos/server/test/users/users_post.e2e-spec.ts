// TODO: fix these rules
/* eslint-disable max-lines, max-statements */
import { Test, TestingModule } from '@nestjs/testing'
import { INestApplication, ValidationPipe } from '@nestjs/common'
import request from './../testReqAgent'
import { UsersController } from '../../src/users/users.controller'
import { USERS_SERVICE } from '../../src/users/interfaces/users.service.interface'
import { createTestApiApplication } from '../../test/testUtils'

describe('UsersController POST (e2e)', () => {
    let app: INestApplication
    const VALID_PASS = 'Aa1!Bb2@'
    const VALID_USERNAME = 'flexgen_user'
    const VALID_ROLE = 'admin'

    beforeEach(async () => {
        const moduleFixture: TestingModule = await Test.createTestingModule({
            controllers: [UsersController],
            providers: [
                {
                    provide: USERS_SERVICE,
                    useValue: {
                        create: jest
                            .fn()
                            .mockImplementation(() => Promise.resolve({ id: 'a uuid' })),
                    },
                },
            ],
        }).compile()

        app = createTestApiApplication(moduleFixture)
        app.useGlobalPipes(new ValidationPipe())
        await app.init()
    })

    it('/users (POST): valid data', () => {
        const requestBody = {
            role: VALID_ROLE,
            password: VALID_PASS,
            username: VALID_USERNAME,
        }

        return request(app.getHttpServer()).post('/users').send(requestBody).expect(201).expect({
            id: 'a uuid',
        })
    })

    it('/users (POST): username empty', () => {
        const requestBody = {
            role: VALID_ROLE,
            password: VALID_PASS,
        }

        return request(app.getHttpServer()).post('/users').send(requestBody).expect(400)
    })

    it('/users (POST): username double underscore', () => {
        const requestBody = {
            username: 'flexgen__user',
            role: VALID_ROLE,
            password: VALID_PASS,
        }

        return request(app.getHttpServer()).post('/users').send(requestBody).expect(400)
    })

    it('/users (POST): username less than min length', () => {
        const requestBody = {
            username: 'flex',
            role: VALID_ROLE,
            password: VALID_PASS,
        }

        return request(app.getHttpServer()).post('/users').send(requestBody).expect(400)
    })

    it('/users (POST): username greater than max length', () => {
        const requestBody = {
            username: 'abcdefghijklmnopqrstuvwxyz',
            role: VALID_ROLE,
            password: VALID_PASS,
        }

        return request(app.getHttpServer()).post('/users').send(requestBody).expect(400)
    })

    it('/users (POST): role is invalid', () => {
        const requestBody = {
            username: VALID_USERNAME,
            role: 'fake_role',
            password: VALID_PASS,
        }

        return request(app.getHttpServer()).post('/users').send(requestBody).expect(400)
    })

    it('/users (POST): password is invalid, too short', () => {
        const requestBody = {
            username: VALID_USERNAME,
            role: VALID_ROLE,
            password: '',
        }
        const responseBody = {
            statusCode: 400,
            message: ['password is invalid'],
            error: 'Bad Request',
        }

        return request(app.getHttpServer())
            .post('/users')
            .send(requestBody)
            .expect(400)
            .expect(responseBody)
    })

    it('/users (POST): password is invalid, missing lowercase', () => {
        const requestBody = {
            username: VALID_USERNAME,
            role: VALID_ROLE,
            password: 'A1!BBBBB',
        }
        const responseBody = {
            statusCode: 400,
            message: ['password is invalid'],
            error: 'Bad Request',
        }

        return request(app.getHttpServer())
            .post('/users')
            .send(requestBody)
            .expect(400)
            .expect(responseBody)
    })

    it('/users (POST): password is invalid, missing uppercase', () => {
        const requestBody = {
            username: VALID_USERNAME,
            role: VALID_ROLE,
            password: 'a1!bbbbb',
        }
        const responseBody = {
            statusCode: 400,
            message: ['password is invalid'],
            error: 'Bad Request',
        }

        return request(app.getHttpServer())
            .post('/users')
            .send(requestBody)
            .expect(400)
            .expect(responseBody)
    })

    it('/users (POST): password is invalid, missing digit', () => {
        const requestBody = {
            username: VALID_USERNAME,
            role: VALID_ROLE,
            password: 'ac!Bbbbb',
        }
        const responseBody = {
            statusCode: 400,
            message: ['password is invalid'],
            error: 'Bad Request',
        }

        return request(app.getHttpServer())
            .post('/users')
            .send(requestBody)
            .expect(400)
            .expect(responseBody)
    })

    it('/users (POST): password is invalid, missing special character', () => {
        const requestBody = {
            username: VALID_USERNAME,
            role: VALID_ROLE,
            password: 'ac5Bbbbb',
        }
        const responseBody = {
            statusCode: 400,
            message: ['password is invalid'],
            error: 'Bad Request',
        }

        return request(app.getHttpServer())
            .post('/users')
            .send(requestBody)
            .expect(400)
            .expect(responseBody)
    })

    it('/users (POST): password is invalid, has control character', () => {
        const requestBody = {
            username: VALID_USERNAME,
            role: VALID_ROLE,
            password: 'ac5	Bbbb!',
        }
        const responseBody = {
            statusCode: 400,
            message: ['password is invalid'],
            error: 'Bad Request',
        }

        return request(app.getHttpServer())
            .post('/users')
            .send(requestBody)
            .expect(400)
            .expect(responseBody)
    })

    afterAll(async () => {
        await app.close()
    })
})
