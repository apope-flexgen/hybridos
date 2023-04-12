import { INestApplication } from '@nestjs/common'
import { WsAdapter } from '@nestjs/platform-ws'
import { Test, TestingModule } from '@nestjs/testing'
import cookieParser from 'cookie-parser'
import { MongoMemoryServer } from 'mongodb-memory-server'

import request from '../../../testReqAgent'
import { Roles } from 'shared/types/api/Users/Users.types'

import { AppModule } from '../../../../src/app/app.module'
import { AppEnvService } from '../../../../src/environment/appEnv.service'
import * as testUtils from '../../../testUtils'
import { hashSync } from 'bcryptjs'

describe('Rest Controller (Integration)', () => {
    let app: INestApplication
    let db: MongoMemoryServer
    let appEnvService: AppEnvService
    const plainTextPass = 'Aa1!Bb2@'
    const INVALID_REST_USER_DATA = {
        role: Roles.Admin,
        password: hashSync(plainTextPass, testUtils.HASH_COST),
        username: 'flexgen_user',
    }

    const VALID_USER_DATA = {
        role: Roles.Rest,
        password: hashSync(plainTextPass, testUtils.HASH_COST),
        username: 'flexgen_rest_user',
    }

    beforeEach(async () => {
        const mongoServer = await MongoMemoryServer.create()

        const moduleFixture: TestingModule = await Test.createTestingModule({
            imports: [AppModule],
        })
            .overrideProvider(AppEnvService)
            .useValue(testUtils.mockAppEnvService(mongoServer.getUri()))
            .compile()

        db = mongoServer

        app = testUtils.createTestApiApplication(moduleFixture)
        appEnvService = moduleFixture.get<AppEnvService>(AppEnvService)
        app.useWebSocketAdapter(new WsAdapter(app))
        app.use(cookieParser())
        await app.init()
    })

    it('should fail basic access authentication: user not found', async () => {
      await testUtils.initializeUser(appEnvService.getMongoUri(), VALID_USER_DATA)
      const encoding = 'Basic ' + Buffer.from("not_user:not_password").toString('base64')
      await request(app.getHttpServer())
        .set('Authorization', encoding)
        .get('/rest/random_route')
        .expect({
            statusCode: 401,
            message: 'Unauthorized'
        })
    });

    it('should fail basic access authentication: invalid role', async () => {
        await testUtils.initializeUser(appEnvService.getMongoUri(), INVALID_REST_USER_DATA)
        const encoding = 'Basic ' + Buffer.from(`${INVALID_REST_USER_DATA.username}:${plainTextPass}`).toString('base64')
        await request(app.getHttpServer())
          .set('Authorization', encoding)
          .get('/rest/random_route')
          .expect({
            statusCode: 403,
            message: 'Forbidden resource',
            error: 'Forbidden'
        })
    });

    it('should pass basic access authentication: valid role and username', async () => {
        await testUtils.initializeUser(appEnvService.getMongoUri(), VALID_USER_DATA)
        const encoding = 'Basic ' + Buffer.from(`${VALID_USER_DATA.username}:${plainTextPass}`).toString('base64')
        const res = await request(app.getHttpServer())
          .set('Authorization', encoding)
          .get('/rest/random_route')
        expect(res.status).toEqual(200)
    });


    afterEach(async () => {
        FIMS.closeConnection()
        await app.close()

        if (db) {
            await db.stop()
        }
    })
})
