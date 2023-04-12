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

const isDefaultsiteAdmins = (res) => {
    testUtils.checksiteAdminsFields(res, testUtils.site(false, false, false))
}

describe('Authentication (Integration)', () => {
    let app: INestApplication
    let db: MongoMemoryServer

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
        app.useWebSocketAdapter(new WsAdapter(app))
        app.use(cookieParser())
        await app.init()
    })

    beforeEach(async () => {
        await testUtils.initializeSite(db.getUri(), testUtils.site(false, false, false))
        await testUtils.initializeUser(db.getUri(), await testUtils.adminUser(false, false))
    })

    it('logs in, test protected route, log out, test protected route', async () => {
        let accessToken: string
        let refreshToken: string

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
                refreshToken = res.headers['set-cookie']
                accessToken = res.body.accessToken
            })

        // test protected route (should pass)
        await request(app.getHttpServer())
            .get('/app-settings')
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
    await testUtils.dropDatabase(db.getUri());
  });

  afterAll(async () => {
    FIMS.closeConnection();
    await app.close();

    if (db) {
      await db.stop();
    }
  });
});
