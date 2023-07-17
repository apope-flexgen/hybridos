import { INestApplication } from '@nestjs/common'
import { JwtModule } from '@nestjs/jwt'
import { Test, TestingModule } from '@nestjs/testing'

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
import { AccessTokenStrategy } from '../../src/auth/strategies/accessToken.strategy'
import { LocalStrategy } from '../../src/auth/strategies/local.strategy'
import { RadiusStrategy } from '../../src/auth/strategies/radius.strategy'
import { TotpStrategy } from '../../src/auth/strategies/totp.strategy'
import { ValidAccessTokenService } from '../../src/auth/validAccessToken.service'
import { UserNotFoundException } from '../../src/users/exceptions/exceptions'
import { RefreshTokenService } from '../../src/auth/refreshTokenService'
import { RefreshTokenStrategy } from '../../src/auth/strategies/refreshToken.strategy'
import { IUsersService, USERS_SERVICE } from '../../src/users/interfaces/users.service.interface'
import * as testUtils from '../testUtils'
import { AppEnvService } from '../../src/environment/appEnv.service'
import { AUDIT_LOGGING_SERVICE } from 'src/logging/auditLogging/interfaces/auditLogging.service.interface'
import path from 'path'

describe('AuthController Login (e2e)', () => {
    let app: INestApplication
    let usersService: IUsersService
    let siteAdminsService: ISiteAdminsService
    const NONEXISTENT_USERNAME = 'userDoesNotExist'
    const INCORRECT_PASS = 'incorrectPassword'

    const USER_NOT_EXPIRED = testUtils.adminUser(false, false)
    const USER_EXPIRED = testUtils.adminUser(true, false)

    const USER_MFA_ENABLED = testUtils.adminUser(false, true)
    const USER_MFA_NOT_ENABLED = testUtils.adminUser(false, false)

    const site = (mfa: boolean, passExp: boolean, radius: boolean) => {
        return {
            password: {
                password_expiration: false,
                minimum_password_length: 8,
                maximum_password_length: 128,
                password_expiration_interval: '8d',
                oldPasswords: 0,
                password_regular_expression: JSON.stringify(
                    /^(?=.*[a-z])(?=.*[A-Z])(?=.*[0-9])(?=.*[!"#\$%&'\*\+,\.\/:;=\\?@\^`\|~])/
                ),
                multi_factor_authentication: false,
            },
            radius: {
                is_enabled: false,
                ip_address: '127.0.0.1',
                port: '1812',
                secret_phrase: 'testing123',
                wait_time: 5000,
                is_local_auth_disabled: false,
            },
        }
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
                    useValue: {
                        authenticate: jest.fn().mockResolvedValue(null),
                    },
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
                {
                    provide: 'WEB_UI_CONFIG_PATH',
                    useValue: path.resolve(__dirname, '../../test/configs/test-web_ui.json')
                },
                {
                    provide: 'WEB_SERVER_CONFIG_PATH',
                    useValue: path.resolve(__dirname, '../../test/configs/test-config.json')
                },
                RadiusStrategy,
                LocalStrategy,
                AccessTokenStrategy,
                RefreshTokenStrategy,
                AccessTokenMfaStrategy,
                TotpStrategy,
                {
                    provide: AUDIT_LOGGING_SERVICE,
                    useValue: {
                        postAuditLog: jest.fn()
                    },
                },
            ],
        }).compile()

        app = testUtils.createTestApiApplication(moduleFixture)
        usersService = moduleFixture.get<IUsersService>(USERS_SERVICE)
        siteAdminsService = moduleFixture.get<ISiteAdminsService>(SITE_ADMINS_SERVICE)
        await app.init()
    })

    describe('/login : localauth', () => {
        it('successfully logs in user', () => {
            const URL = `/login`

            const requestBody = {
                username: testUtils.VALID_USERNAME,
                password: testUtils.VALID_PASS,
            }
            jest.spyOn(usersService, 'readByUsername').mockResolvedValue(USER_NOT_EXPIRED)
            jest.spyOn(siteAdminsService, 'find').mockResolvedValue(testUtils.site(false, false, false))
            return request(app.getHttpServer())
                .post(URL)
                .send(requestBody)
                .then((res) => {
                    expect(res.status).toBe(200)
                    expect(res.body).toEqual(expect.objectContaining(testUtils.SUCCESS_RESPONSE))
                    expect(res.headers['set-cookie']).toBeTruthy()
                })
        })

        it('unsuccessful (incorrect password)', () => {
            const URL = `/login`

            const requestBody = {
                username: testUtils.VALID_USERNAME,
                password: INCORRECT_PASS,
            }
            jest.spyOn(usersService, 'readByUsername').mockResolvedValue(USER_NOT_EXPIRED)
            return request(app.getHttpServer())
                .post(URL)
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

        it('unsuccessful (user does not exist)', () => {
            const URL = `/login`

            const requestBody = {
                username: NONEXISTENT_USERNAME,
                password: INCORRECT_PASS,
            }
            jest.spyOn(usersService, 'readByUsername').mockImplementationOnce(() => {
                throw new UserNotFoundException()
            })
            return request(app.getHttpServer())
                .post(URL)
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

    describe('/login : MFA Enabled', () => {
        it('returns successful mfa response (user has not enabled mfa)', () => {
            const URL = `/login`

            const requestBody = {
                username: testUtils.VALID_USERNAME,
                password: testUtils.VALID_PASS,
            }
            jest.spyOn(usersService, 'readByUsername').mockResolvedValue(USER_MFA_NOT_ENABLED)
            jest.spyOn(siteAdminsService, 'find').mockResolvedValue(testUtils.site(true, false, false))
            return request(app.getHttpServer())
                .post(URL)
                .send(requestBody)
                .then((res) => {
                    expect(res.status).toBe(200)
                    expect(res.body.username).toBe(testUtils.VALID_USERNAME)
                    expect(res.body).toHaveProperty('qrCode')
                    expect(res.headers['set-cookie']).toBeFalsy()
                })
        })

        it('returns successful mfa response (user has enabled mfa)', () => {
            const URL = `/login`

            const requestBody = {
                username: testUtils.VALID_USERNAME,
                password: testUtils.VALID_PASS,
            }
            jest.spyOn(usersService, 'readByUsername').mockResolvedValue(USER_MFA_ENABLED)
            jest.spyOn(siteAdminsService, 'find').mockResolvedValue(testUtils.site(true, false, false))
            return request(app.getHttpServer())
                .post(URL)
                .send(requestBody)
                .then((res) => {
                    expect(res.status).toBe(200)
                    expect(res.body.username).toBe(testUtils.VALID_USERNAME)
                    expect(res.body).not.toHaveProperty('qrCode')
                    expect(res.headers['set-cookie']).toBeFalsy()
                })
        })
    })

    describe('/login : password expiration', () => {
        it('successfully logs in user (password not expired)', () => {
            const URL = `/login`

            const requestBody = {
                username: testUtils.VALID_USERNAME,
                password: testUtils.VALID_PASS,
            }

            jest.spyOn(usersService, 'readByUsername').mockResolvedValue(USER_NOT_EXPIRED)
            jest.spyOn(siteAdminsService, 'find').mockResolvedValue(testUtils.site(false, true, false))
            return request(app.getHttpServer())
                .post(URL)
                .send(requestBody)
                .then((res) => {
                    expect(res.status).toBe(200)
                    expect(res.body).toEqual(expect.objectContaining(testUtils.SUCCESS_RESPONSE))
                    expect(res.headers['set-cookie']).toBeTruthy()
                })
        })

        it('fails (password is expired)', () => {
            const URL = `/login`

            const requestBody = {
                username: testUtils.VALID_USERNAME,
                password: testUtils.VALID_PASS,
            }
            jest.spyOn(usersService, 'readByUsername').mockResolvedValue(USER_EXPIRED)
            jest.spyOn(siteAdminsService, 'find').mockResolvedValue(testUtils.site(false, true, false))
            return request(app.getHttpServer())
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
                    expect(res.headers['set-cookie']).toBeFalsy()
                })
        })
    })
})
