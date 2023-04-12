import { INestApplication } from '@nestjs/common'
import { Test, TestingModule } from '@nestjs/testing'

import request from './testReqAgent'

import { createTestApiApplication } from './testUtils'
import { SITE_ADMINS_SERVICE } from '../src/siteAdmins/interfaces/siteAdmin.service.interface'
import { SiteAdminsController } from '../src/siteAdmins/siteAdmins.controller'
import { SiteAdminsDto } from '../src/siteAdmins/dto/create-siteAdmins.dto'

describe('SiteAdmins (e2e)', () => {
    let app: INestApplication
    const defaultSiteAdmins = {
        password: {
            password_expiration: false,
            minimum_password_length: 8,
            maximum_password_length: 128,
            password_expiration_interval: '8d',
            old_passwords: 0,
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

    beforeEach(async () => {
        const moduleFixture: TestingModule = await Test.createTestingModule({
            controllers: [SiteAdminsController],
            providers: [
                {
                    provide: SITE_ADMINS_SERVICE,
                    useValue: {
                        find: jest.fn().mockResolvedValue(defaultSiteAdmins),
                        create: jest.fn().mockImplementation((siteAdmin: SiteAdminsDto) => {
                            return Promise.resolve({ ...siteAdmin })
                        }),
                    },
                },
            ],
        }).compile()

        app = createTestApiApplication(moduleFixture)
        await app.init()
    })

    it('/GET app-settings', () => {
        return request(app.getHttpServer())
            .get('/app-settings')
            .expect(200)
            .expect(defaultSiteAdmins)
    })

    it('/POST app-settings', () => {
        return request(app.getHttpServer())
            .post('/app-settings')
            .send(defaultSiteAdmins)
            .expect(201)
            .expect(defaultSiteAdmins)
    })

    afterAll(async () => {
        await app.close()
    })
})
