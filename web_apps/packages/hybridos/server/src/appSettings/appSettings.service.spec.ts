import { getModelToken } from '@nestjs/mongoose'
import { Test } from '@nestjs/testing'
import { Model } from 'mongoose'

import { AppSettingsService } from './appSettings.service'
import { AppSetting } from './interfaces/appSetting.interface'
import {
    APP_SETTINGS_SERVICE,
    IAppSettingsService,
} from './interfaces/appSetting.service.interface'
import { AppSettingDocument } from './schemas/appSetting.schema'

describe('AppSettingsController', () => {
    let service: IAppSettingsService
    let model: Model<AppSettingDocument>
    const defaultAppSettings: AppSetting = {
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

    const mockAppSetting = (): AppSetting => defaultAppSettings

    beforeEach(async () => {
        const module = await Test.createTestingModule({
            providers: [
                {
                    provide: APP_SETTINGS_SERVICE,
                    useClass: AppSettingsService,
                },
                {
                    provide: getModelToken('appsettings'),
                    useValue: {
                        new: jest.fn().mockResolvedValue(mockAppSetting()),
                        constructor: jest.fn().mockResolvedValue(mockAppSetting()),
                        findOne: jest.fn(),
                        create: jest.fn(),
                        exec: jest.fn(),
                    },
                },
            ],
        }).compile()

        service = module.get<IAppSettingsService>(APP_SETTINGS_SERVICE)
        model = module.get<Model<AppSettingDocument>>(getModelToken('appsettings'))
    })

    it('should be defined', () => {
        expect(service).toBeDefined()
    })

    afterEach(() => {
        jest.clearAllMocks()
    })

    // find tests

    it('should return default appSettings (empty database)', async () => {
        jest.spyOn(model, 'findOne').mockReturnValue({
            exec: jest.fn().mockResolvedValueOnce(undefined),
        } as any)
        jest.spyOn(model, 'create').mockReturnValue(mockAppSetting() as any)
        const appSetting = await service.find()
        expect(appSetting).toEqual(defaultAppSettings)
    })

    it('should return current appSettings (non-empty database)', async () => {
        jest.spyOn(model, 'findOne').mockReturnValue({
            exec: jest.fn().mockResolvedValueOnce(mockAppSetting()),
        } as any)
        const appSetting = await service.find()
        expect(appSetting).toEqual(defaultAppSettings)
    })

    // create tests

    it('should insert a new appSettings', async () => {
        jest.spyOn(model, 'create').mockReturnValue(mockAppSetting() as any)
        const appSetting = await service.create(mockAppSetting())
        expect(appSetting).toEqual(defaultAppSettings)
    })
})
