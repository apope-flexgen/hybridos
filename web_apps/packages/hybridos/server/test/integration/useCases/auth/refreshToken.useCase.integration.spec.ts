import { INestApplication, Module } from '@nestjs/common'
import { WsAdapter } from '@nestjs/platform-ws'
import { Test, TestingModule } from '@nestjs/testing'
// import * as cookieParser from 'cookie-parser';
import cookieParser from 'cookie-parser'
import { MongoMemoryServer } from 'mongodb-memory-server'

import request from './../../../testReqAgent'

import { AppModule } from '../../../../src/app.module'
import { AppEnvService } from '../../../../src/environment/appEnv.service'
import * as testUtils from '../../../testUtils'

describe('Authentication (Integration)', () => {
    let app: INestApplication
    let db: MongoMemoryServer
    let appEnvService: AppEnvService

    beforeAll(async () => {
        const mongoServer = await MongoMemoryServer.create()
        let config = testUtils.mockAppEnvService(
            mongoServer.getUri(),
            jest.fn().mockImplementation(() => {
                // 3 seconds for testing
                return 3
            })
        )

        const moduleFixture: TestingModule = await Test.createTestingModule({
            imports: [AppModule],
        })
            .overrideProvider(AppEnvService)
            .useValue(config)
            .compile()

        db = mongoServer

        app = testUtils.createTestApiApplication(moduleFixture)
        appEnvService = moduleFixture.get(AppEnvService)
        app.useWebSocketAdapter(new WsAdapter(app))
        app.use(cookieParser())
        await app.init()
    })

    beforeEach(async () => {
        await testUtils.initializeSite(db.getUri(), testUtils.site(false, false, false))
        await testUtils.initializeUser(db.getUri(), await testUtils.adminUser(false, false))
    })

    it('/login should set httpOnly, Secure, SameSite refreshToken as Cookie', async () => {
        let cookies: string[]
        let keys: string[]

        const requestBody = {
            username: testUtils.VALID_USERNAME,
            password: testUtils.VALID_PASS,
        }

        // login
        await request(app.getHttpServer())
            .post('/login')
            .send(requestBody)
            .then((res) => {
                expect(res.status).toBe(200)
                expect(res.body).toEqual(expect.objectContaining(testUtils.SUCCESS_RESPONSE))
                expect(res.headers['set-cookie']).toBeTruthy()
                cookies = res.headers['set-cookie']
                keys = cookies[0].split('; ')
                expect(keys).toEqual(
                    expect.arrayContaining(['HttpOnly', 'Secure', 'SameSite=Strict'])
                )
            })
        return
    })

    it('access token should expire after 3 seconds', async () => {
        let accessToken: string
        let refreshToken: string
        const expectedError = { statusCode: 401, message: 'jwt expired' }
        const waitMs = 3000

        const requestBody = {
            username: testUtils.VALID_USERNAME,
            password: testUtils.VALID_PASS,
        }

        // login
        await request(app.getHttpServer())
            .post('/login')
            .send(requestBody)
            .then((res) => {
                expect(res.status).toBe(200)
                expect(res.body).toEqual(expect.objectContaining(testUtils.SUCCESS_RESPONSE))
                expect(res.body.accessToken).toBeTruthy()
                refreshToken = res.headers['set-cookie']
                accessToken = res.body.accessToken
            })

        await new Promise((f) => setTimeout(f, waitMs))
        await request(app.getHttpServer())
            .get('/app-settings')
            .set('Authorization', accessToken)
            .set('Cookie', refreshToken)
            .then((res) => {
                expect(res.body).toStrictEqual(expectedError)
                expect(res.status).toBe(401)
            })
        return
    })

    it('get new accessToken and refreshToken', async () => {
        let initialAccessToken: string
        let initialRefreshToken: string
        let newAccessToken: string
        let newRefreshToken: string

        const requestBody = {
            username: testUtils.VALID_USERNAME,
            password: testUtils.VALID_PASS,
        }

        // login
        await request(app.getHttpServer())
            .post('/login')
            .send(requestBody)
            .then((res) => {
                expect(res.status).toBe(200)
                expect(res.body).toEqual(expect.objectContaining(testUtils.SUCCESS_RESPONSE))
                expect(res.body.accessToken).toBeTruthy()
                initialRefreshToken = res.headers['set-cookie']
                initialAccessToken = res.body.accessToken
            })

        // wait for 3 second expiration
        await new Promise((f) => setTimeout(f, 3000))
        await request(app.getHttpServer())
            .get('/app-settings')
            .set('Authorization', initialAccessToken)
            .then((res) => {
                expect(res.status).toBe(401)
            })

        await request(app.getHttpServer())
            .get('/refresh_token')
            .set('Cookie', initialRefreshToken)
            .then((res) => {
                expect(res.status).toBe(200)
                expect(res.body.accessToken).toBeTruthy()
                expect(res.headers['set-cookie']).toBeTruthy()
                newRefreshToken = res.headers['set-cookie']
                newAccessToken = res.body.accessToken
                expect(newRefreshToken).not.toEqual(initialRefreshToken)
                expect(newAccessToken).not.toEqual(initialAccessToken)
            })
        return
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
