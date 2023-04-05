import { ConfigService } from '@nestjs/config'
import { Test, TestingModule } from '@nestjs/testing'
import * as fs from 'fs'

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
            ],
        }).compile()

        const appEnvService = module.get<AppEnvService>(AppEnvService)
        expect(appEnvService.getAppServerPort()).toEqual(9999)
        expect(appEnvService.getMongoName()).toEqual('db_name')
        expect(appEnvService.getMongoURL()).toEqual('db_url')
        expect(appEnvService.getHTTPTimeout()).toEqual(5000)
        expect(appEnvService.getRefreshTokenTimeout()).toEqual(28800)
    })
})
