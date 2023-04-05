import { INestApplication } from '@nestjs/common'
import { JwtService } from '@nestjs/jwt'
import { WsAdapter } from '@nestjs/platform-ws'
import { Test, TestingModule } from '@nestjs/testing'
// import * as cookieParser from 'cookie-parser';
import cookieParser from 'cookie-parser'
import { ObjectId } from 'mongodb'
import { MongoMemoryServer } from 'mongodb-memory-server'
import { authenticator } from 'otplib'

import request from './../../../testReqAgent'

import { AppModule } from '../../../../src/app.module'
import { AppEnvService } from '../../../../src/environment/appEnv.service'
import * as testUtils from '../../../testUtils'

let jwtService: JwtService
let MFA_ACCESS_TOKEN: string
let INVALID_MFA_ACCESS_TOKEN: string

const initializeAccessToken = async (userID: ObjectId) => {
    const payload = {
        username: testUtils.VALID_USERNAME,
        oneTime: 'mfa',
        sub: userID,
    }
    MFA_ACCESS_TOKEN = `Bearer ${jwtService.sign(payload, {
        expiresIn: '12d',
        secret: 'supersecretkey-oneTimeUse-mfa',
    })}`
}

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

        jwtService = moduleFixture.get<JwtService>(JwtService)
        INVALID_MFA_ACCESS_TOKEN = `Bearer ${jwtService.sign(
            {},
            {
                secret: 'incorrectsignature',
            }
        )}`

        await app.init()
    })

    beforeEach(async () => {
        await testUtils.initializeSite(db.getUri(), testUtils.site(true, false, false))
        await testUtils
            .initializeUser(db.getUri(), await testUtils.adminUser(false, false))
            .then((userID) => {
                initializeAccessToken(userID)
            })
    })

    describe('/login/mfa responses', () => {
        const URL = '/login/mfa'

        it('successfully logs in user', () => {
            const CURRENT_TOTP = authenticator.generate(testUtils.VALID_KEY)
            const mfaRequestBody = {
                username: testUtils.VALID_USERNAME,
                totp: CURRENT_TOTP,
            }

            return request(app.getHttpServer())
                .post(URL)
                .set('Authorization', MFA_ACCESS_TOKEN)
                .send(mfaRequestBody)
                .then((res) => {
                    expect(res.status).toBe(200)
                    expect(res.body).toEqual(expect.objectContaining(testUtils.SUCCESS_RESPONSE))
                    expect(res.headers['set-cookie']).toBeTruthy()
                })
        })

        it('fails (incorrect code)', () => {
            const CURRENT_TOTP = authenticator.generate(testUtils.VALID_KEY)
            const INVALID_TOTP = CURRENT_TOTP.split('').reverse().join('')
            const mfaRequestBody = {
                username: testUtils.VALID_USERNAME,
                totp: INVALID_TOTP,
            }

            return request(app.getHttpServer())
                .post(URL)
                .set('Authorization', MFA_ACCESS_TOKEN)
                .send(mfaRequestBody)
                .then((res) => {
                    expect(res.status).toBe(401)
                    expect(res.body).toStrictEqual({
                        statusCode: 401,
                        message: 'Incorrect TOTP Code',
                    })
                    expect(res.headers['set-cookie']).toBeFalsy()
                })
        })

        it('fails (invalid accessToken)', () => {
            const CURRENT_TOTP = authenticator.generate(testUtils.VALID_KEY)
            const mfaRequestBody = {
                username: testUtils.VALID_USERNAME,
                totp: CURRENT_TOTP,
            }

            return request(app.getHttpServer())
                .post(URL)
                .set('Authorization', INVALID_MFA_ACCESS_TOKEN)
                .send(mfaRequestBody)
                .then((res) => {
                    expect(res.status).toBe(401)
                    expect(res.body).toStrictEqual({
                        statusCode: 401,
                        message: 'Unauthorized',
                    })
                    expect(res.headers['set-cookie']).toBeFalsy()
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
