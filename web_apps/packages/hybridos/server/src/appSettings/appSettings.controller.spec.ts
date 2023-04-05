import { Test } from '@nestjs/testing'

import { AppSettingsController } from './appSettings.controller'
import { CreateAppSettingsDto } from './dto/create-appSettings.dto'
import { APP_SETTINGS_SERVICE } from './interfaces/appSetting.service.interface'

describe('AppSettingsController', () => {
    let controller: AppSettingsController
    const defaultAppSettings: CreateAppSettingsDto = {
        password: {
            multi_factor_authentication: false,
            password_expiration: false,
            minimum_password_length: 8,
            maximum_password_length: 128,
            password_expiration_interval: '8d',
            old_passwords: 0,
            password_regular_expression: JSON.stringify(
                /^(?=.*[a-z])(?=.*[A-Z])(?=.*[0-9])(?=.*[!"#\$%&'\*\+,\.\/:;=\\?@\^`\|~])/
            ),

            lowercase: true,
            uppercase: true,
            digit: true,
            special: true,
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
        const module = await Test.createTestingModule({
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

        controller = module.get<AppSettingsController>(AppSettingsController)
    })

    it('should be defined', () => {
        expect(controller).toBeDefined()
    })

    describe('find', () => {
        it('should return the current appSettings', async () => {
            expect(controller.find()).resolves.toEqual(defaultAppSettings)
        })
    })

    describe('create', () => {
        it('should return the inserted appSettings', async () => {
            expect(await controller.create(defaultAppSettings)).toEqual({
                ...defaultAppSettings,
            })
        })
    })
})
