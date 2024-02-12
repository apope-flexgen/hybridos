import { ConfigService } from '@nestjs/config';
import { Test, TestingModule } from '@nestjs/testing';
import * as fs from 'fs';
import path from 'path';

import { AppEnvService } from './appEnv.service';
import { APP_ENV_SERVICE, IAppEnvService } from 'src/environment/appEnv.interface';
import { SECRET_GENERATION_SERVICE } from 'src/environment/secretGeneration/interfaces/secretGeneration.interface';

describe('AppEnvService', () => {
  it('AppEnvService.appConfig should be loaded with config data.', async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [
        { provide: APP_ENV_SERVICE, useClass: AppEnvService },
        {
          provide: ConfigService,
          useValue: {
            get: jest.fn().mockReturnValue('some_path_value'),
          },
        },
        {
          provide: 'WEB_UI_CONFIG_PATH',
          useValue: path.resolve(__dirname, '../../test/configs/test-web_ui.json'),
        },
        {
          provide: 'WEB_SERVER_CONFIG_PATH',
          useValue: path.resolve(__dirname, '../../test/configs/test-config.json'),
        },
        {
          provide: SECRET_GENERATION_SERVICE,
          useValue: { generateSecret: jest.fn().mockReturnValue('secretString') },
        },
      ],
    }).compile();

    const appEnvService = module.get<IAppEnvService>(APP_ENV_SERVICE);
    expect(appEnvService.getAppServerPort()).toEqual(443);
    expect(appEnvService.getMongoName()).toEqual('hybridos_authentication');
    expect(appEnvService.getMongoURL()).toEqual('mongodb://localhost:27017');
    expect(appEnvService.getHttpTimeout()).toEqual(10000);
    expect(appEnvService.getRefreshTokenTimeout()).toEqual(28800);
    expect(appEnvService.getJwtSecretKey()).toEqual('secretString');
  });
});
