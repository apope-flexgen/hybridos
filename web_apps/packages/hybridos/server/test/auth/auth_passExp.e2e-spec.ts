import { INestApplication, ValidationPipe } from '@nestjs/common'
import { JwtModule, JwtService } from '@nestjs/jwt'
import { Test, TestingModule } from '@nestjs/testing'
// import * as cookieParser from 'cookie-parser';
import cookieParser from 'cookie-parser'

import request from './../testReqAgent'

import {
    SITE_ADMINS_SERVICE,
    ISiteAdminsService,
} from '../../src/siteAdmins/interfaces/siteAdmin.service.interface'
import { AuthController } from '../../src/auth/auth.controller'
import { AuthService } from '../../src/auth/auth.service'
import { AUTH_SERVICE } from '../../src/auth/interfaces/auth.service.interface'
import { MFA_SERVICE } from '../../src/auth/interfaces/mfa.service.interface'
import { PASS_EXP_SERVICE } from '../../src/auth/interfaces/passExp.service.interface'
import { RADIUS_SERVICE } from '../../src/auth/interfaces/radius.service.interface'
import { VALID_JWT_SERVICE } from '../../src/auth/interfaces/validJWT.service.interface'
import { MfaService } from '../../src/auth/mfa.service'
import { PassExpService } from '../../src/auth/passExp.service'
import { AccessTokenMfaStrategy } from '../../src/auth/strategies/accessToken.mfa.strategy'
import { AccessTokenPassExpStrategy } from '../../src/auth/strategies/accessToken.passExp.strategy'
import { AccessTokenStrategy } from '../../src/auth/strategies/accessToken.strategy'
import { LocalStrategy } from '../../src/auth/strategies/local.strategy'
import { TotpStrategy } from '../../src/auth/strategies/totp.strategy'
import { ValidAccessTokenService } from '../../src/auth/validAccessToken.service'
import { RefreshTokenService } from '../../src/auth/refreshTokenService'
import { RefreshTokenStrategy } from '../../src/auth/strategies/refreshToken.strategy'
import { AppEnvService } from '../../src/environment/appEnv.service'
import { IUsersService, USERS_SERVICE } from '../../src/users/interfaces/users.service.interface'
import * as testUtils from '../testUtils'

describe('AuthController Login - PassExp (e2e)', () => {
    let app: INestApplication
    let usersService: IUsersService
    let siteAdminsService: ISiteAdminsService
    let jwtService: JwtService

    const URL = '/login/passExp'

    const VALID_ID = 'user_id'
    const OLD_PASS = testUtils.VALID_PASS
    const INVALID_PASS = 'bad'

    const USER_PASS_EXPIRED = testUtils.adminUser(true, false)
    const USER_PASS_NOT_EXPIRED = testUtils.adminUser(false, false)

    const USER_MFA_NOT_ENABLED = testUtils.adminUser(true, false)
    const USER_MFA_ENABLED = testUtils.adminUser(true, true)

    const site = (mfa: boolean, passExp: boolean) => {
        return {
            password: {
                password_expiration: passExp,
                minimum_password_length: 8,
                maximum_password_length: 128,
                password_expiration_interval: '8d',
                old_passwords: 0,
                password_regular_expression: JSON.stringify(
                    /^(?=.*[a-z])(?=.*[A-Z])(?=.*[0-9])(?=.*[!"#\$%&'\*\+,\.\/:;=\\?@\^`\|~])/
                ),
                multi_factor_authentication: mfa,
            },
            radius: {
                is_enabled: false,
                ip_address: '127.0.0.1',
                port: '1812',
                secret_phrase: 'secretphrase',
                wait_time: 5000,
                is_local_auth_disabled: false
            },
        }
    }

    let VALID_ACCESS_TOKEN: string
    let INVALID_ACCESS_TOKEN: string
    const payload = {
        username: testUtils.VALID_USERNAME,
        oneTime: 'password-expiration',
        sub: VALID_ID,
    }

    beforeAll(async () => {
        const moduleFixture: TestingModule = await Test.createTestingModule({
            imports: [
                JwtModule.register({
                    secret: 'supersecretkey',
                }),
            ],
            controllers: [AuthController],
            providers: [
                {
                    provide: USERS_SERVICE,
                    useValue: {
                        readByUsername: jest.fn(),
                        update: jest.fn().mockResolvedValue(USER_PASS_NOT_EXPIRED),
                    },
                },
                {
                    provide: SITE_ADMINS_SERVICE,
                    useValue: {
                        find: jest.fn(),
                    },
                },
                {
                    provide: RADIUS_SERVICE,
                    useValue: {},
                },
                {
                    provide: AUTH_SERVICE,
                    useClass: AuthService,
                },
                {
                    provide: MFA_SERVICE,
                    useClass: MfaService,
                },
                {
                    provide: PASS_EXP_SERVICE,
                    useClass: PassExpService,
                },
                {
                    provide: VALID_JWT_SERVICE,
                    useClass: ValidAccessTokenService,
                },
                RefreshTokenService,
                AppEnvService,
                RefreshTokenStrategy,
                LocalStrategy,
                AccessTokenStrategy,
                AccessTokenMfaStrategy,
                AccessTokenPassExpStrategy,
                TotpStrategy,
            ],
        }).compile()

        app = testUtils.createTestApiApplication(moduleFixture)
        usersService = moduleFixture.get<IUsersService>(USERS_SERVICE)
        siteAdminsService = moduleFixture.get<ISiteAdminsService>(SITE_ADMINS_SERVICE)

        jwtService = moduleFixture.get<JwtService>(JwtService)
        VALID_ACCESS_TOKEN = `Bearer ${jwtService.sign(payload, {
            expiresIn: '12d',
            secret: 'supersecretkey-oneTimeUse-password-expiration',
        })}`
        INVALID_ACCESS_TOKEN = `Bearer ${jwtService.sign(payload, {
            expiresIn: '12d',
            secret: 'invalidSecret',
        })}`

        app.useGlobalPipes(new ValidationPipe({ whitelist: true }))
        app.use(cookieParser())
        await app.init()
    })

    describe('/login/passExp', () => {
        it('successfully logs in user', () => {
            const requestBody = {
                username: testUtils.VALID_USERNAME,
                updatedPassword: testUtils.NEW_PASS,
            }

            jest.spyOn(usersService, 'readByUsername').mockResolvedValue(USER_PASS_EXPIRED)
            jest.spyOn(siteAdminsService, 'find').mockResolvedValue(site(false, true))
            return request(app.getHttpServer())
                .post(URL)
                .set('Authorization', VALID_ACCESS_TOKEN)
                .send(requestBody)
                .then((res) => {
                    expect(res.status).toBe(200)
                    expect(res.body).toEqual(expect.objectContaining(testUtils.SUCCESS_RESPONSE))
                    expect(res.headers['set-cookie']).toBeTruthy()
                })
        })

        it('fails (password was not updated)', () => {
            const requestBody = {
                username: testUtils.VALID_USERNAME,
                updatedPassword: OLD_PASS,
            }

            jest.spyOn(usersService, 'readByUsername').mockResolvedValue(USER_PASS_EXPIRED)
            jest.spyOn(siteAdminsService, 'find').mockResolvedValue(site(false, true))
            return request(app.getHttpServer())
                .post(URL)
                .set('Authorization', VALID_ACCESS_TOKEN)
                .send(requestBody)
                .then((res) => {
                    expect(res.status).toBe(400)
                    expect(res.body).toStrictEqual({
                        statusCode: 400,
                        message: 'New password must not match current password',
                    })
                    expect(res.headers['set-cookie']).toBeFalsy()
                })
        })

        it('fails (invalid password)', () => {
            const requestBody = {
                username: testUtils.VALID_USERNAME,
                updatedPassword: INVALID_PASS,
            }

            jest.spyOn(usersService, 'readByUsername').mockResolvedValue(USER_PASS_EXPIRED)
            jest.spyOn(siteAdminsService, 'find').mockResolvedValue(site(false, true))
            return request(app.getHttpServer())
                .post(URL)
                .set('Authorization', VALID_ACCESS_TOKEN)
                .send(requestBody)
                .then((res) => {
                    expect(res.status).toBe(400)
                    expect(res.body).toStrictEqual({
                        error: 'Bad Request',
                        message: ['password is invalid'],
                        statusCode: 400,
                    })
                    expect(res.headers['set-cookie']).toBeFalsy()
                })
        })

        it('fails (invalid accessToken)', () => {
            const requestBody = {
                username: testUtils.VALID_USERNAME,
                updatedPassword: testUtils.NEW_PASS,
            }

            return request(app.getHttpServer())
                .post(URL)
                .set('Authorization', INVALID_ACCESS_TOKEN)
                .send(requestBody)
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

    describe('/login/passExp - mfa enabled', () => {
        it('returns successful mfa response (user has not enabled mfa)', () => {
            const requestBody = {
                username: testUtils.VALID_USERNAME,
                updatedPassword: testUtils.NEW_PASS,
            }
            jest.spyOn(usersService, 'readByUsername').mockResolvedValue(USER_MFA_NOT_ENABLED)
            jest.spyOn(siteAdminsService, 'find').mockResolvedValue(site(true, true))
            return request(app.getHttpServer())
                .post(URL)
                .set('Authorization', VALID_ACCESS_TOKEN)
                .send(requestBody)
                .then((res) => {
                    expect(res.status).toBe(200)
                    expect(res.body.username).toBe(testUtils.VALID_USERNAME)
                    expect(res.body).toHaveProperty('qrCode')
                    expect(res.headers['set-cookie']).toBeFalsy()
                })
        })

        it('returns successful mfa response (user has enabled mfa)', () => {
            const requestBody = {
                username: testUtils.VALID_USERNAME,
                updatedPassword: testUtils.NEW_PASS,
            }
            jest.spyOn(usersService, 'readByUsername').mockResolvedValue(USER_MFA_ENABLED)
            jest.spyOn(siteAdminsService, 'find').mockResolvedValue(site(true, true))
            return request(app.getHttpServer())
                .post(URL)
                .set('Authorization', VALID_ACCESS_TOKEN)
                .send(requestBody)
                .then((res) => {
                    // expect(res.status).toBe(200);
                    expect(res.body).toEqual(
                        expect.objectContaining({
                            username: testUtils.VALID_USERNAME,
                            mfaRequired: true,
                        })
                    )
                    expect(res.header['set-cookie']).toBeFalsy()
                })
        })
    })
})
