import { INestApplication } from '@nestjs/common'
import { WsAdapter } from '@nestjs/platform-ws'
import { Test, TestingModule } from '@nestjs/testing'
// import * as cookieParser from 'cookie-parser';
import cookieParser from 'cookie-parser'
import { MongoMemoryServer } from 'mongodb-memory-server'

import request from './../../../testReqAgent'

import { AppModule } from '../../../../src/app.module'
import { AppEnvService } from '../../../../src/environment/appEnv.service'
import * as testUtils from '../../../testUtils'
import { ValidPasswordConstraint } from 'src/users/validators/IsValidPassword'
import { SITE_ADMINS_SERVICE } from 'src/siteAdmins/interfaces/siteAdmin.service.interface'
import { useContainer } from 'class-validator'
import { PermissionsService } from 'src/permissions/permissions.service'

const INCORRECT_PASSWORD = 'incorrectPassword'
const INCORRECT_USERNAME = 'userDoesntExist'

describe('Authentication (Integration)', () => {
    let app: INestApplication
    let db: MongoMemoryServer

    beforeEach(async () => {
        const mongoServer = await MongoMemoryServer.create()

        const moduleFixture: TestingModule = await Test.createTestingModule({
            imports: [AppModule],
            providers: [
                ValidPasswordConstraint,
                {
                    provide: SITE_ADMINS_SERVICE,
                    useValue: {
                        find: jest.fn()
                    }
                },
            ]
        })
            .overrideProvider(AppEnvService)
            .useValue(testUtils.mockAppEnvService(mongoServer.getUri()))
            .overrideProvider(PermissionsService)
            .useValue({webServerConfigDirectoryPath: () => ''})
            .compile()

        db = mongoServer

        app = testUtils.createTestApiApplication(moduleFixture)
        useContainer(moduleFixture, { fallbackOnErrors: true })
        app.useWebSocketAdapter(new WsAdapter(app))
        app.use(cookieParser())
        await app.init()
    })

    beforeEach(async () => {
        await testUtils.initializeSite(db.getUri(), testUtils.site(false, true, false))
    })

    describe('/login responses', () => {
        const URL = '/login'

        describe('password is expired', () => {
            beforeEach(async () => {
                await testUtils.initializeUser(db.getUri(), await testUtils.adminUser(true, false))
            })

            it('should return password expired response', async () => {
                const requestBody = {
                    username: testUtils.VALID_USERNAME,
                    password: testUtils.VALID_PASS,
                }

                return await request(app.getHttpServer())
                    .post(URL)
                    .send(requestBody)
                    .then((res) => {
                        expect(res.status).toBe(200)
                        expect(res.body).toEqual(
                            expect.objectContaining({
                                username: testUtils.VALID_USERNAME,
                                passwordExpired: true,
                            })
                        )
                        expect(res.body.accessToken).toBeTruthy()
                    })
            })

            it('should fail to log in user (invalid password)', async () => {
                const requestBody = {
                    username: testUtils.VALID_USERNAME,
                    password: INCORRECT_PASSWORD,
                }

                return await request(app.getHttpServer())
                    .post(URL)
                    .send(requestBody)
                    .expect(401)
                    .expect({
                        statusCode: 401,
                        message: 'Unauthorized',
                    })
            })

            it('should fail to log in user (cannot find user)', async () => {
                const requestBody = {
                    username: INCORRECT_USERNAME,
                    password: testUtils.VALID_PASS,
                }

                return await request(app.getHttpServer())
                    .post(URL)
                    .send(requestBody)
                    .expect(401)
                    .expect({
                        statusCode: 401,
                        message: 'Unauthorized',
                    })
            })
        })

        describe('password is not yet expired', () => {
            beforeEach(async () => {
                await testUtils.initializeUser(db.getUri(), await testUtils.adminUser(false, false))
            })

            it('should return successful login response', async () => {
                const requestBody = {
                    username: testUtils.VALID_USERNAME,
                    password: testUtils.VALID_PASS,
                }

                return await request(app.getHttpServer())
                    .post(URL)
                    .send(requestBody)
                    .then((res) => {
                        expect(res.status).toBe(200)
                        expect(res.body).toEqual(
                            expect.objectContaining(testUtils.SUCCESS_RESPONSE)
                        )
                        expect(res.headers['set-cookie']).toBeTruthy()
                    })
            })

            it('should fail to log in user (invalid password)', async () => {
                const requestBody = {
                    username: testUtils.VALID_USERNAME,
                    password: INCORRECT_PASSWORD,
                }

                return await request(app.getHttpServer())
                    .post(URL)
                    .send(requestBody)
                    .expect(401)
                    .expect({
                        statusCode: 401,
                        message: 'Unauthorized',
                    })
            })

            it('should fail to log in user (cannot find user)', async () => {
                const requestBody = {
                    username: INCORRECT_USERNAME,
                    password: testUtils.VALID_PASS,
                }

                return await request(app.getHttpServer())
                    .post(URL)
                    .send(requestBody)
                    .expect(401)
                    .expect({
                        statusCode: 401,
                        message: 'Unauthorized',
                    })
            })
        })
    })

    afterEach(async () => {
        await testUtils.dropDatabase(db.getUri())
    })

    afterAll(async () => {
        FIMS.closeConnection()
        await app.close()

        if (db) {
            await db.stop()
        }
    })
})
