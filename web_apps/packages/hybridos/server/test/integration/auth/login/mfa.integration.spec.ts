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

const INCORRECT_PASSWORD = 'incorrectPassword'
const INCORRECT_USERNAME = 'userDoesntExist'

describe('Authentication (Integration)', () => {
    let app: INestApplication
    let db: MongoMemoryServer

    beforeAll(async () => {
        const mongoServer = await MongoMemoryServer.create()

        const moduleFixture: TestingModule = await Test.createTestingModule({
            imports: [AppModule],
        })
            .overrideProvider(AppEnvService)
            .useValue(testUtils.mockAppEnvService(mongoServer.getUri()))
            .compile()

        db = mongoServer

        app = testUtils.createTestApiApplication(moduleFixture)
        app.useWebSocketAdapter(new WsAdapter(app))
        app.use(cookieParser())
        await app.init()
    })

    beforeEach(async () => {
        await testUtils.initializeSite(db.getUri(), testUtils.site(true, false, false))
        await testUtils.initializeUser(db.getUri(), await testUtils.adminUser(false, false))
    })

    describe('/login responses', () => {
        const URL = '/login'

        it('should return mfa response (not yet enabled)', async () => {
            const requestBody = {
                username: testUtils.VALID_USERNAME,
                password: testUtils.VALID_PASS,
            }

            return await request(app.getHttpServer())
                .post(URL)
                .send(requestBody)
                .then((res) => {
                    expect(res.status).toBe(200)
                    expect(res.body.username).toBe(testUtils.VALID_USERNAME)
                    expect(res.body).toHaveProperty('qrCode')
                })
        })

        it('should return mfa response (already enabled)', async () => {
            await testUtils.initializeUser(db.getUri(), await testUtils.adminUser(false, true))

            const requestBody = {
                username: testUtils.VALID_USERNAME,
                password: testUtils.VALID_PASS,
            }

            return await request(app.getHttpServer())
                .post(URL)
                .send(requestBody)
                .then((res) => {
                    expect(res.status).toBe(200)
                    expect(res.body.username).toBe(testUtils.VALID_USERNAME)
                    expect(res.body).not.toHaveProperty('qrCode')
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
