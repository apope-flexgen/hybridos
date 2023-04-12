import { INestApplication } from '@nestjs/common'
import { JwtModule } from '@nestjs/jwt'
import { Test, TestingModule } from '@nestjs/testing'
import { hashSync } from 'bcryptjs'

import request from './../testReqAgent'

import { SITE_ADMINS_SERVICE } from '../../src/siteAdmins/interfaces/siteAdmin.service.interface'
import { AuthController } from '../../src/auth/auth.controller'
import { AuthService } from '../../src/auth/auth.service'
import { AUTH_SERVICE } from '../../src/auth/interfaces/auth.service.interface'
import { MFA_SERVICE } from '../../src/auth/interfaces/mfa.service.interface'
import { PASS_EXP_SERVICE } from '../../src/auth/interfaces/passExp.service.interface'
import { IRadiusService, RADIUS_SERVICE } from '../../src/auth/interfaces/radius.service.interface'
import { VALID_JWT_SERVICE } from '../../src/auth/interfaces/validJWT.service.interface'
import { MfaService } from '../../src/auth/mfa.service'
import { PassExpService } from '../../src/auth/passExp.service'
import { AccessTokenMfaStrategy } from '../../src/auth/strategies/accessToken.mfa.strategy'
import { AccessTokenStrategy } from '../../src/auth/strategies/accessToken.strategy'
import { LocalStrategy } from '../../src/auth/strategies/local.strategy'
import { RadiusStrategy } from '../../src/auth/strategies/radius.strategy'
import { TotpStrategy } from '../../src/auth/strategies/totp.strategy'
import { ValidAccessTokenService } from '../../src/auth/validAccessToken.service'
import { USERS_SERVICE } from '../../src/users/interfaces/users.service.interface'
import { RefreshTokenService } from '../../src/auth/refreshTokenService'
import { RefreshTokenStrategy } from '../../src/auth/strategies/refreshToken.strategy'
import { AppEnvService } from '../../src/environment/appEnv.service'
import { createTestApiApplication, HASH_COST } from '../testUtils'

describe('AuthController Login (e2e)', () => {
    let app: INestApplication
    let radiusService: IRadiusService
    const LOCAL_USERNAME = 'user1'
    const RADIUS_USERNAME = 'radiusUsername'
    const VALID_ID = 'user_id'
    const LOCAL_ROLE = 'user'
    const RADIUS_ROLE = 'Admin'
    const LOCAL_PASS = 'P@ssw0rd'
    const RADIUS_PASS = 'radiusPassword'
    const HASHED_PASS = hashSync(LOCAL_PASS, HASH_COST)

    const user = (pwdUpdatedDate: Date, mfaEnabled: boolean) => {
        return {
            id: VALID_ID,
            role: LOCAL_ROLE,
            username: LOCAL_USERNAME,
            password: HASHED_PASS,
            pwdUpdatedDate: pwdUpdatedDate,
            mfa_enabled: mfaEnabled,
        }
    }

    const LOCAL_USER = user(new Date(Date.now()), false)

    const site = (mfa: boolean, passExp: boolean, radius: boolean) => {
        return {
            password: {
                multi_factor_authenticatoin: mfa,
                password_expiration: passExp,
                minimum_password_length: 8,
                maximum_password_length: 128,
                password_expiration_interval: '8d',
                old_passwords: 0,
                password_regular_expression: JSON.stringify(
                    /^(?=.*[a-z])(?=.*[A-Z])(?=.*[0-9])(?=.*[!"#\$%&'\*\+,\.\/:;=\\?@\^`\|~])/
                ),
            },
            radius: {
                is_enabled: radius,
                ip_address: '127.0.0.1',
                port: '1812',
                secret_phrase: 'testing123',
                wait_time: 5000,
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
                        readByUsername: jest.fn().mockResolvedValue(LOCAL_USER),
                    },
                },
                {
                    provide: SITE_ADMINS_SERVICE,
                    useValue: {
                        find: jest.fn().mockResolvedValue(site(false, false, true)),
                    },
                },
                {
                    provide: RADIUS_SERVICE,
                    useValue: {
                        authenticate: jest.fn(),
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
                RefreshTokenStrategy,
                RadiusStrategy,
                LocalStrategy,
                AccessTokenStrategy,
                AccessTokenMfaStrategy,
                TotpStrategy,
            ],
        }).compile()

        app = createTestApiApplication(moduleFixture)
        radiusService = moduleFixture.get<IRadiusService>(RADIUS_SERVICE)
        await app.init()
    })

    describe('/login : Radius Enabled', () => {
        it('successfully logs in with radius', () => {
            const URL = `/login`

            const requestBody = {
                username: RADIUS_USERNAME,
                password: RADIUS_PASS,
            }

            jest.spyOn(radiusService, 'authenticate').mockResolvedValue(RADIUS_ROLE)

            return request(app.getHttpServer())
                .post(URL)
                .send(requestBody)
                .then((res) => {
                    expect(res.status).toBe(200)
                    expect(res.body).toEqual(
                        expect.objectContaining({
                            role: RADIUS_ROLE,
                            username: RADIUS_USERNAME,
                        })
                    )
                    expect(res.headers['set-cookie']).toBeTruthy()
                })
        })

        it('fails to log in (invalid radius credentials)', () => {
            const URL = `/login`

            const requestBody = {
                username: RADIUS_USERNAME,
                password: RADIUS_PASS,
            }

            jest.spyOn(radiusService, 'authenticate').mockResolvedValue(null)

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

        it('successfully logs in with local after radius fails', () => {
            const URL = `/login`

            const requestBody = {
                username: LOCAL_USERNAME,
                password: LOCAL_PASS,
            }

            jest.spyOn(radiusService, 'authenticate').mockResolvedValue(null)

            return request(app.getHttpServer())
                .post(URL)
                .send(requestBody)
                .then((res) => {
                    expect(res.status).toBe(200)
                    expect(res.body).toEqual(
                        expect.objectContaining({
                            role: LOCAL_ROLE,
                            username: LOCAL_USERNAME,
                        })
                    )
                    expect(res.headers['set-cookie']).toBeTruthy()
                })
        })
    })
})
