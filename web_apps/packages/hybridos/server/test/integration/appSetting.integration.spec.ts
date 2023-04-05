import { INestApplication } from '@nestjs/common'
import { WsAdapter } from '@nestjs/platform-ws'
import { Test, TestingModule } from '@nestjs/testing'
// import * as cookieParser from 'cookie-parser';
import cookieParser from 'cookie-parser'
import { MongoClient } from 'mongodb'
import { MongoMemoryServer } from 'mongodb-memory-server'

import request from './../testReqAgent'
import { User } from '../../../shared/types/dtos/auth.dto'

import { AppModule } from '../../src/app.module'
import { AppEnvService } from '../../src/environment/appEnv.service'
import * as testUtils from '../testUtils'

let VALID_ACCESS_TOKEN: string
const VALID_USERNAME = 'user1'
const VALID_PASS = 'P@ssw0rd'

const defaultAppSettings = {
    password: {
        password_expiration: false,
        minimum_password_length: 8,
        maximum_password_length: 128,
        password_expiration_interval: '8d',
        old_passwords: 0,
        password_regular_expression: JSON.stringify(
            /^(?=.*[a-z])(?=.*[A-Z])(?=.*[0-9])(?=.*[!"#\$%&'\*\+,\.\/:;=\\?@\^`\|~])/
        ),
        multi_factor_authentication: false
    },
    radius: {
        is_enabled: false,
        ip_address: '127.0.0.1',
        port: '1812',
        secret_phrase: 'testing123',
        wait_time: 5000,
        is_local_auth_disabled: false
    },
}

const altAppSettings = {
    password: {
        password_expiration: false,
        minimum_password_length: 15,
        maximum_password_length: 128,
        password_expiration_interval: '24h',
        old_passwords: 3,
        password_regular_expression: JSON.stringify(
            /^(?=.*[a-z])(?=.*[A-Z])(?=.*[0-9])(?=.*[!"#\$%&'\*\+,\.\/:;=\\?@\^`\|~])/
        ),
        multi_factor_authentication: false,
    },
    radius: {
        is_enabled: true,
        ip_address: '127.0.0.1',
        port: '1234',
        secret_phrase: 'testing123',
        wait_time: 64,
        is_local_auth_disabled: false
    },
}

const invalidAppSettings = {
    password: {
        password_expiration: false,
        minimum_password_length: 2,
        maximum_password_length: 2000,
        password_expiration_interval: '127hours',
        old_passwords: -3,
        password_regular_expression: '',
        multi_factor_authentication: null,
    },
    radius: {
        is_enabled: true,
        ip_address: 'notAnIpAddress',
        port: 'shipWindow',
        secret_phrase: 'testing123',
        wait_time: -304,
        is_local_auth_disabled: false,
    },
}

const isAltAppSettings = (res) => {
    testUtils.checkAppSettingsFields(res, altAppSettings)
}
const isDefaultAppSettings = (res) => {
    testUtils.checkAppSettingsFields(res, defaultAppSettings)
}

const initializeUser = async (uri: string, user: User) => {
    const con = await MongoClient.connect(uri, {})
    const tdb = con.db('integrationTestDb')

    expect(tdb).toBeDefined()
    const col = tdb.collection('users')
    await col.deleteMany({})
    expect(await col.countDocuments({})).toBe(0)
    const result = await col.insertMany([{ ...user }])
    expect(result.insertedCount).toStrictEqual(1)
    await con.close()
}

describe('AppSettings (Integration)', () => {
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

    beforeAll(async () => {
        VALID_ACCESS_TOKEN = await testUtils.generateAdminAccessToken(app, db)
    })

    it('should return default appSettings with empty db', async () => {
        console.log('accessToken: ', VALID_ACCESS_TOKEN)
        return await request(app.getHttpServer())
            .get('/app-settings')
            .set('Authorization', `${VALID_ACCESS_TOKEN}`)
            .expect(200)
            .expect(isDefaultAppSettings)
    })

    it('should return updated appSettings', async () => {
        return await request(app.getHttpServer())
            .post('/app-settings')
            .set('Authorization', `${VALID_ACCESS_TOKEN}`)
            .send(altAppSettings)
            .expect(201)
            .expect(isAltAppSettings)
    })

    it('should return current appSettings', async () => {
        await request(app.getHttpServer())
            .get('/app-settings')
            .set('Authorization', `${VALID_ACCESS_TOKEN}`)
            .expect(200)
            .expect(isDefaultAppSettings)

        await request(app.getHttpServer())
            .post('/app-settings')
            .set('Authorization', `${VALID_ACCESS_TOKEN}`)
            .send(altAppSettings)
            .expect(201)
            .expect(isAltAppSettings)
        const con = await MongoClient.connect(db.getUri(), {})
        const col = con.db('integrationTestDb').collection('appsettings')
        console.log('app settings documents num: ', await col.countDocuments({}))
        await con.close()

        return await request(app.getHttpServer())
            .get('/app-settings')
            .set('Authorization', `${VALID_ACCESS_TOKEN}`)
            .expect(200)
            .expect(isAltAppSettings)
    })

    it('should fail due to invalid input', async () => {
        return await request(app.getHttpServer())
            .post('/app-settings')
            .set('Authorization', `${VALID_ACCESS_TOKEN}`)
            .send(invalidAppSettings)
            .expect(400)
            .then((res) => {
                expect(res.body).toStrictEqual({
                    statusCode: 400,
                    error: 'Bad Request',
                    message: [
                        'password.multi_factor_authentication must be a boolean value',
                        'password.minimum_password_length must not be less than 8',
                        'password.maximum_password_length must not be greater than 1028',
                        'password.Invalid password expiration interval, interval must be a positive integer followed by one of [d, m, h]',
                        'password.old_passwords must not be less than 0',
                        'radius.ip_address must be an ip address',
                        'radius.port must be a port',
                        'radius.wait_time must be a positive number',
                    ],
                })
            })
    })

    it('testing capped capability', async () => {
        await request(app.getHttpServer())
            .post('/app-settings')
            .set('Authorization', `${VALID_ACCESS_TOKEN}`)
            .send(altAppSettings)
            .expect(201)
            .expect(isAltAppSettings)

        await request(app.getHttpServer())
            .post('/app-settings')
            .set('Authorization', `${VALID_ACCESS_TOKEN}`)
            .send(altAppSettings)
            .expect(201)
            .expect(isAltAppSettings)

        await request(app.getHttpServer())
            .post('/app-settings')
            .set('Authorization', `${VALID_ACCESS_TOKEN}`)
            .send(altAppSettings)
            .expect(201)
            .expect(isAltAppSettings)

        await request(app.getHttpServer())
            .post('/app-settings')
            .set('Authorization', `${VALID_ACCESS_TOKEN}`)
            .send(altAppSettings)
            .expect(201)
            .expect(isAltAppSettings)

        const con = await MongoClient.connect(db.getUri(), {})
        const col = con.db('integrationTestDb').collection('appsettings')
        console.log('iscapped: ', await col.isCapped())
        expect(await col.countDocuments({})).toBe(1)
        await con.close()
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
