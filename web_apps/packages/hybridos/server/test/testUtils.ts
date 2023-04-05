import { RequestMethod } from '@nestjs/common'
import { TestingModule } from '@nestjs/testing'
import { hash } from 'bcryptjs'
import { MongoClient, ObjectId } from 'mongodb'
import { authenticator } from 'otplib'
import request from './testReqAgent'
import { Roles } from '../../shared/types/api/Users/Users.types'
import { User } from '../../shared/types/dtos/auth.dto'

import { AppSetting } from '../src/appSettings/interfaces/appSetting.interface'

export const HTTP_TIMEOUT = 15000
export const ACCESS_TOKEN_SECRET_FIMS_SOCKET = 'test_secret'
export const REFRESH_TOKEN_TIMEOUT = 28800 // seconds
export const ACCESS_TOKEN_TIMEOUT = 180 // seconds

export const VALID_USERNAME = 'user1'
export const VALID_PASS = 'P@ssw0rd1'
export const NEW_PASS = 'P@ssw0rd2'
export const ADMIN_ROLE = Roles.Admin
export const USER_ROLE = Roles.User
export const VALID_KEY = authenticator.generateSecret()
export const HASH_COST = 10

export const SUCCESS_RESPONSE = {
    username: VALID_USERNAME,
    role: ADMIN_ROLE,
}

export const FORBIDDEN_RESPONSE = {
    statusCode: 403,
    message: 'Forbidden resource',
    error: 'Forbidden',
}

export const createTestApiApplication = (moduleFixture: TestingModule) => {
    const app = moduleFixture.createNestApplication()
    app.setGlobalPrefix('api', {
        exclude: [{path: 'rest', method: RequestMethod.ALL}]
    })
    return app
  }

export const site = (mfa: boolean, passExp: boolean, radius: boolean) => {
    return {
        password: {
            multi_factor_authentication: mfa,
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
            secret_phrase: 'secretphrase',
            wait_time: 5000,
            is_local_auth_disabled: false,
        },
    }
}

export const adminUser = async (pwdExpired: boolean, mfaEnabled: boolean) => {
    return await userTemplate(pwdExpired, mfaEnabled, ADMIN_ROLE)
}

export const userUser = async (pwdExpired: boolean, mfaEnabled: boolean) => {
    return await userTemplate(pwdExpired, mfaEnabled, USER_ROLE)
}

export const mockAppEnvService = (mongoUri: string, accessTokenGetter?: jest.Mock<any, any>) => {
    return {
        getMongoUri: jest.fn().mockImplementation(() => {
            return `${mongoUri}integrationTestDb`
        }),
        getHttpTimeout: jest.fn().mockImplementation(() => {
            return HTTP_TIMEOUT
        }),
        getAccessTokenSecretFimsSocket: jest.fn().mockImplementation(() => {
            return ACCESS_TOKEN_SECRET_FIMS_SOCKET
        }),
        getAccessTokenTimeout:
            accessTokenGetter ||
            jest.fn().mockImplementation(() => {
                return ACCESS_TOKEN_TIMEOUT
            }),
        getRefreshTokenTimeout: jest.fn().mockImplementation(() => {
            return REFRESH_TOKEN_TIMEOUT
        }),
    }
}

const userTemplate = async (pwdExpired: boolean, mfaEnabled: boolean, role: Roles) => {
    return {
        role: role,
        username: VALID_USERNAME,
        password: await hash(VALID_PASS, HASH_COST),
        pwdUpdatedDate: pwdExpired ? new Date(0) : new Date(Date.now()),
        mfa_enabled: mfaEnabled,
        shared_key: VALID_KEY,
    }
}

export const checkAppSettingsFields = (res, app) => {
    for (const property in app) {
        expect(res.body[property]).toBeDefined()
        for (const subProperty in app[property]) {
            expect(res.body[property][subProperty]).toBe(app[property][subProperty])
        }
    }
}

export const initializeSite = async (uri: string, site: AppSetting) => {
    const con = await MongoClient.connect(uri, {})
    const tdb = con.db('integrationTestDb')

    expect(tdb).toBeDefined()
    const col = tdb.collection('appsettings')
    const result = await col.insertMany([{ ...site }])
    expect(result.insertedCount).toStrictEqual(1)
    expect(await col.countDocuments({})).toBe(1)
    await con.close()
}

export const initializeUser = async (uri: string, user: User): Promise<ObjectId> => {
    const con = await MongoClient.connect(uri, {})
    const tdb = con.db('integrationTestDb')

    expect(tdb).toBeDefined()
    const col = tdb.collection('users')
    await col.deleteMany({})
    expect(await col.countDocuments({})).toBe(0)
    const result = await col.insertOne({ ...user })

    await con.close()

    return result.insertedId
}

export const dropDatabase = async (url: string) => {
    const con = await MongoClient.connect(url, {})
    await con.db('integrationTestDb').dropDatabase()
    await con.close()
}

export const generateAdminAccessToken = async (app, db): Promise<string> => {
    return await accessTokenTemplate(app, db, ADMIN_ROLE)
}

export const generateUserAccessToken = async (app, db): Promise<string> => {
    return await accessTokenTemplate(app, db, USER_ROLE)
}

export const accessTokenTemplate = async (app, db, role: Roles): Promise<string> => {
    let VALID_ACCESS_TOKEN: string
    await initializeUser(db.getUri(), {
        username: VALID_USERNAME,
        role: role,
        password: await hash(VALID_PASS, HASH_COST),
    })

    const loginRequest = {
        username: VALID_USERNAME,
        password: VALID_PASS,
    }

    await request(app.getHttpServer())
        .post('/login')
        .send(loginRequest)
        .then((res) => {
            expect(res.status).toBe(200)
            expect(res.body).toEqual(
                expect.objectContaining({
                    username: VALID_USERNAME,
                    role: role,
                })
            )
            expect(res.headers['set-cookie']).toBeTruthy()
            VALID_ACCESS_TOKEN = res.body.accessToken
        })
    return VALID_ACCESS_TOKEN
}
