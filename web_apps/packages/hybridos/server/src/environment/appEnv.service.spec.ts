import { ConfigService } from '@nestjs/config'
import { Test, TestingModule } from '@nestjs/testing'
import * as fs from 'fs'
import path from 'path'

import { AppEnvService } from './appEnv.service'

describe('AppEnvService', () => {
    it('AppEnvService.appConfig should be loaded with config data.', async () => {
        const module: TestingModule = await Test.createTestingModule({
            providers: [
                AppEnvService,
                {
                    provide: ConfigService,
                    useValue: {
                        get: jest.fn().mockReturnValue('some_path_value'),
                    },
                },
                {
                    provide: 'WEB_UI_CONFIG_PATH',
                    useValue: path.resolve(__dirname, '../../test/configs/test-web_ui.json')
                },
                {
                    provide: 'WEB_SERVER_CONFIG_PATH',
                    useValue: path.resolve(__dirname, '../../test/configs/test-config.json')
                },
            ],
        }).compile()

        const appEnvService = module.get<AppEnvService>(AppEnvService)
        expect(appEnvService.getAppServerPort()).toEqual(443)
        expect(appEnvService.getMongoName()).toEqual('hybridos_authentication')
        expect(appEnvService.getMongoURL()).toEqual('mongodb://localhost:27017')
        expect(appEnvService.getHttpTimeout()).toEqual(10000)
        expect(appEnvService.getRefreshTokenTimeout()).toEqual(28800)
    })
})
