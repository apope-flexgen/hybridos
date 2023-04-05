import { INestApplication } from '@nestjs/common'
import { Test, TestingModule } from '@nestjs/testing'

import request from './testReqAgent'

import { createTestApiApplication } from './testUtils'
import { APP_SETTINGS_SERVICE } from '../src/appSettings/interfaces/appSetting.service.interface'
import { AppSettingsController } from '../src/appSettings/appSettings.controller'
import { CreateAppSettingsDto } from '../src/appSettings/dto/create-appSettings.dto'

describe('AppSettings (e2e)', () => {
    let app: INestApplication
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
            controllers: [AppSettingsController],
            providers: [
                {
                    provide: APP_SETTINGS_SERVICE,
                    useValue: {
                        find: jest.fn().mockResolvedValue(defaultAppSettings),
                        create: jest.fn().mockImplementation((appSetting: CreateAppSettingsDto) => {
                            return Promise.resolve({ ...appSetting })
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
            .expect(defaultAppSettings)
    })

    it('/POST app-settings', () => {
        return request(app.getHttpServer())
            .post('/app-settings')
            .send(defaultAppSettings)
            .expect(201)
            .expect(defaultAppSettings)
    })

    afterAll(async () => {
        await app.close()
    })
})
