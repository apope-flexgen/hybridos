import { INestApplication } from '@nestjs/common'
import { WsAdapter } from '@nestjs/platform-ws'
import { Test, TestingModule } from '@nestjs/testing'
// import * as cookieParser from 'cookie-parser';
import cookieParser from 'cookie-parser'
import { MongoMemoryServer } from 'mongodb-memory-server'
import { authenticator } from 'otplib'

import request from './../../../testReqAgent'

import { AppModule } from '../../../../src/app.module'
import { AppEnvService } from '../../../../src/environment/appEnv.service'
import * as testUtils from '../../../testUtils'
import { ValidPasswordConstraint } from 'src/users/validators/IsValidPassword'
import { SITE_ADMINS_SERVICE } from 'src/siteAdmins/interfaces/siteAdmin.service.interface'
import { useContainer } from 'class-validator'
import { PermissionsService } from 'src/permissions/permissions.service'

const isDefaultsiteAdmins = (res) => {
    testUtils.checksiteAdminsFields(res, testUtils.site(false, false, false))
}

describe('Authentication (Integration)', () => {
    let app: INestApplication
    let db: MongoMemoryServer

    beforeAll(async () => {
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
        await testUtils.initializeSite(db.getUri(), testUtils.site(true, true, false))
    })

    it('logs in after resetting password and mfa (enabled), test protected route, log out, test protected route', async () => {
        await testUtils.initializeUser(db.getUri(), await testUtils.adminUser(true, true))

        let accessToken: string
        let refreshToken: string
        let passExpAccessToken: string
        let mfaAccessToken: string

        const requestBody = {
            username: testUtils.VALID_USERNAME,
            password: testUtils.VALID_PASS,
        }

        // login (receive password-expired response)
        await request(app.getHttpServer())
            .post('/login')
            .send(requestBody)
            .then((res) => {
                expect(res.status).toBe(200)
                expect(res.body).toEqual(
                    expect.objectContaining({
                        username: testUtils.VALID_USERNAME,
                        passwordExpired: true,
                    })
                )
                passExpAccessToken = res.body.accessToken
            })

        // test protected route (should fail)
        await request(app.getHttpServer())
            .get('/authenticate-user-token')
            .set('Authorization', passExpAccessToken)
            .then((res) => {
                expect(res.status).toBe(401)
                expect(res.body).toStrictEqual({
                    statusCode: 401,
                    message: 'invalid signature',
                })
            })

        // reset password and get mfa response
        const passExpRequestBody = {
            username: testUtils.VALID_USERNAME,
            updatedPassword: testUtils.NEW_PASS,
        }

        await request(app.getHttpServer())
            .post('/login/passExp')
            .set('Authorization', passExpAccessToken)
            .send(passExpRequestBody)
            .then((res) => {
                expect(res.status).toBe(200)
                expect(res.body.username).toBe(testUtils.VALID_USERNAME)
                expect(res.body).not.toHaveProperty('qrCode')
                mfaAccessToken = res.body.accessToken
            })

        // test protected route (should fail)
        await request(app.getHttpServer())
            .get('/authenticate-user-token')
            .set('Authorization', mfaAccessToken)
            .then((res) => {
                expect(res.status).toBe(401)
                expect(res.body).toStrictEqual({
                    statusCode: 401,
                    message: 'invalid signature',
                })
            })

        // login with mfa totp code
        const CURRENT_TOTP = authenticator.generate(testUtils.VALID_KEY)
        const mfaRequestBody = {
            username: testUtils.VALID_USERNAME,
            totp: CURRENT_TOTP,
        }

        await request(app.getHttpServer())
            .post('/login/mfa')
            .set('Authorization', mfaAccessToken)
            .send(mfaRequestBody)
            .then((res) => {
                expect(res.status).toBe(200)
                expect(res.body).toEqual(expect.objectContaining(testUtils.SUCCESS_RESPONSE))
                expect(res.headers['set-cookie']).toBeTruthy()
                accessToken = res.body.accessToken
                refreshToken = res.headers['set-cookie']
            })

        // test protected route (should pass)
        await request(app.getHttpServer())
            .get('/site-admins')
            .set('Authorization', accessToken)
            .then((res) => {
                expect(res.status).toBe(200)
                expect(isDefaultsiteAdmins)
            })

        // logout
        await request(app.getHttpServer())
            .post('/logout')
            .set('Authorization', accessToken)
            .set('Cookie', refreshToken)
            .then((res) => {
                expect(res.status).toBe(200)
                expect(res.body).toStrictEqual({ message: 'user has been logged out' })
            })

        // test protected route (should fail)
        await request(app.getHttpServer())
            .get('/authenticate-user-token')
            .set('Authorization', accessToken)
            .then((res) => {
                expect(res.status).toBe(401)
                expect(res.body).toStrictEqual({
                    statusCode: 401,
                    message: 'Unauthorized',
                })
            })

        return
    })

    it('logs in after resetting password and mfa (not enabled), test protected route, log out, test protected route', async () => {
        await testUtils.initializeUser(db.getUri(), await testUtils.adminUser(true, false))

        let accessToken: string
        let refreshToken: string
        let passExpAccessToken: string
        let mfaAccessToken: string

        const requestBody = {
            username: testUtils.VALID_USERNAME,
            password: testUtils.VALID_PASS,
        }

        // login (receive password-expired response)
        await request(app.getHttpServer())
            .post('/login')
            .send(requestBody)
            .then((res) => {
                expect(res.status).toBe(200)
                expect(res.body).toEqual(
                    expect.objectContaining({
                        username: testUtils.VALID_USERNAME,
                        passwordExpired: true,
                    })
                )
                passExpAccessToken = res.body.accessToken
            })

        // test protected route (should fail)
        await request(app.getHttpServer())
            .get('/authenticate-user-token')
            .set('Authorization', passExpAccessToken)
            .then((res) => {
                expect(res.status).toBe(401)
                expect(res.body).toStrictEqual({
                    statusCode: 401,
                    message: 'invalid signature',
                })
            })

        // reset password and get mfa response
        const passExpRequestBody = {
            username: testUtils.VALID_USERNAME,
            updatedPassword: testUtils.NEW_PASS,
        }

        await request(app.getHttpServer())
            .post('/login/passExp')
            .set('Authorization', passExpAccessToken)
            .send(passExpRequestBody)
            .then((res) => {
                expect(res.status).toBe(200)
                expect(res.body.username).toBe(testUtils.VALID_USERNAME)
                expect(res.body).toHaveProperty('qrCode')
                mfaAccessToken = res.body.accessToken
            })

        // test protected route (should fail)
        await request(app.getHttpServer())
            .get('/authenticate-user-token')
            .set('Authorization', mfaAccessToken)
            .then((res) => {
                expect(res.status).toBe(401)
                expect(res.body).toStrictEqual({
                    statusCode: 401,
                    message: 'invalid signature',
                })
            })

        // login with mfa totp code
        const CURRENT_TOTP = authenticator.generate(testUtils.VALID_KEY)
        const mfaRequestBody = {
            username: testUtils.VALID_USERNAME,
            totp: CURRENT_TOTP,
        }

        await request(app.getHttpServer())
            .post('/login/mfa')
            .set('Authorization', mfaAccessToken)
            .send(mfaRequestBody)
            .then((res) => {
                expect(res.status).toBe(200)
                expect(res.body).toEqual(expect.objectContaining(testUtils.SUCCESS_RESPONSE))
                expect(res.headers['set-cookie']).toBeTruthy()
                accessToken = res.body.accessToken
                refreshToken = res.headers['set-cookie']
            })

        // test protected route (should pass)
        await request(app.getHttpServer())
            .get('/site-admins')
            .set('Authorization', accessToken)
            .then((res) => {
                expect(res.status).toBe(200)
                expect(isDefaultsiteAdmins)
            })

        // logout
        await request(app.getHttpServer())
            .post('/logout')
            .set('Authorization', accessToken)
            .set('Cookie', refreshToken)
            .then((res) => {
                expect(res.status).toBe(200)
                expect(res.body).toStrictEqual({ message: 'user has been logged out' })
            })

        // test protected route (should fail)
        await request(app.getHttpServer())
            .get('/authenticate-user-token')
            .set('Authorization', accessToken)
            .then((res) => {
                expect(res.status).toBe(401)
                expect(res.body).toStrictEqual({
                    statusCode: 401,
                    message: 'Unauthorized',
                })
            })

        return
    })

    it('logs in with unexpired password & mfa (enabled), test protected route, log out, test protected route', async () => {
        await testUtils.initializeUser(db.getUri(), await testUtils.adminUser(false, true))

        let accessToken: string
        let refreshToken: string
        let oneTimeAccessToken: string

        const requestBody = {
            username: testUtils.VALID_USERNAME,
            password: testUtils.VALID_PASS,
        }

        // login (receive mfa response)
        await request(app.getHttpServer())
            .post('/login')
            .send(requestBody)
            .then((res) => {
                expect(res.status).toBe(200)
                expect(res.body.username).toBe(testUtils.VALID_USERNAME)
                expect(res.body).not.toHaveProperty('qrCode')
                oneTimeAccessToken = res.body.accessToken
            })

        // test protected route (should fail)
        await request(app.getHttpServer())
            .get('/authenticate-user-token')
            .set('Authorization', oneTimeAccessToken)
            .then((res) => {
                expect(res.status).toBe(401)
                expect(res.body).toStrictEqual({
                    statusCode: 401,
                    message: 'invalid signature',
                })
            })

        // login with mfa totp code
        const CURRENT_TOTP = authenticator.generate(testUtils.VALID_KEY)
        const mfaRequestBody = {
            username: testUtils.VALID_USERNAME,
            totp: CURRENT_TOTP,
        }

        await request(app.getHttpServer())
            .post('/login/mfa')
            .set('Authorization', oneTimeAccessToken)
            .send(mfaRequestBody)
            .then((res) => {
                expect(res.status).toBe(200)
                expect(res.body).toEqual(expect.objectContaining(testUtils.SUCCESS_RESPONSE))
                expect(res.headers['set-cookie']).toBeTruthy()
                accessToken = res.body.accessToken
                refreshToken = res.headers['set-cookie']
            })

        // test protected route (should pass)
        await request(app.getHttpServer())
            .get('/site-admins')
            .set('Authorization', accessToken)
            .set('Cookie', refreshToken)
            .then((res) => {
                expect(res.status).toBe(200)
                expect(isDefaultsiteAdmins)
            })

        // logout
        await request(app.getHttpServer())
            .post('/logout')
            .set('Authorization', accessToken)
            .set('Cookie', refreshToken)
            .then((res) => {
                expect(res.status).toBe(200)
                expect(res.body).toStrictEqual({ message: 'user has been logged out' })
            })

        // test protected route (should fail)
        await request(app.getHttpServer())
            .get('/authenticate-user-token')
            .set('Authorization', accessToken)
            .then((res) => {
                expect(res.status).toBe(401)
                expect(res.body).toStrictEqual({
                    statusCode: 401,
                    message: 'Unauthorized',
                })
            })

        return
    })

    it('logs in with unexpired password & mfa (not enabled), test protected route, log out, test protected route', async () => {
        await testUtils.initializeUser(db.getUri(), await testUtils.adminUser(false, false))

        let accessToken: string
        let refreshToken: string
        let oneTimeAccessToken: string

        const requestBody = {
            username: testUtils.VALID_USERNAME,
            password: testUtils.VALID_PASS,
        }

        // login (receive mfa response)
        await request(app.getHttpServer())
            .post('/login')
            .send(requestBody)
            .then((res) => {
                expect(res.status).toBe(200)
                expect(res.body.username).toBe(testUtils.VALID_USERNAME)
                expect(res.body).toHaveProperty('qrCode')
                oneTimeAccessToken = res.body.accessToken
            })

        // test protected route (should fail)
        await request(app.getHttpServer())
            .get('/authenticate-user-token')
            .set('Authorization', oneTimeAccessToken)
            .then((res) => {
                expect(res.status).toBe(401)
                expect(res.body).toStrictEqual({
                    statusCode: 401,
                    message: 'invalid signature',
                })
            })

        // login with mfa totp code
        const CURRENT_TOTP = authenticator.generate(testUtils.VALID_KEY)
        const mfaRequestBody = {
            username: testUtils.VALID_USERNAME,
            totp: CURRENT_TOTP,
        }

        await request(app.getHttpServer())
            .post('/login/mfa')
            .set('Authorization', oneTimeAccessToken)
            .send(mfaRequestBody)
            .then((res) => {
                expect(res.status).toBe(200)
                expect(res.body).toEqual(expect.objectContaining(testUtils.SUCCESS_RESPONSE))
                expect(res.headers['set-cookie']).toBeTruthy()
                accessToken = res.body.accessToken
                refreshToken = res.headers['set-cookie']
            })

        // test protected route (should pass)
        await request(app.getHttpServer())
            .get('/site-admins')
            .set('Authorization', accessToken)
            .set('Cookie', refreshToken)
            .then((res) => {
                expect(res.status).toBe(200)
                expect(isDefaultsiteAdmins)
            })

        // logout
        await request(app.getHttpServer())
            .post('/logout')
            .set('Authorization', accessToken)
            .set('Cookie', refreshToken)
            .then((res) => {
                expect(res.status).toBe(200)
                expect(res.body).toStrictEqual({ message: 'user has been logged out' })
            })

        // test protected route (should fail)
        await request(app.getHttpServer())
            .get('/authenticate-user-token')
            .set('Authorization', accessToken)
            .then((res) => {
                expect(res.status).toBe(401)
                expect(res.body).toStrictEqual({
                    statusCode: 401,
                    message: 'Unauthorized',
                })
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
