import { Test, TestingModule } from '@nestjs/testing';
import { IAssetsService } from './assets.interface';
import { DBI_SERVICE, DBI_URIs, IDBIService } from '../dbi/dbi.interface';
import { AssetsService } from './assets.service';
import { ASSETS_RESPONSE, ASSETS_POST } from './assets.mocks';

describe('AssetsService', () => {
  let assetsService: IAssetsService;
  let dbiService: IDBIService;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [
        AssetsService,
        {
          provide: DBI_SERVICE,
          useValue: {
            getFromDBI: jest.fn(),
            postToDBI: jest.fn(),
          },
        },
      ],
    }).compile();

    assetsService = module.get<AssetsService>(AssetsService);
    dbiService = module.get<IDBIService>(DBI_SERVICE);
  });

  it('dbiService should be defined', () => {
    expect(dbiService).toBeDefined();
  });

  describe('test getAssets', () => {
    it('should return assets', async () => {
      jest.spyOn(dbiService, 'getFromDBI').mockResolvedValue(ASSETS_RESPONSE);
      expect(await assetsService.getAssets()).toMatchObject(ASSETS_RESPONSE);
    });
  });

  describe('test postAssets', () => {
    it('should use the data body', async () => {
      const postAssetsToDBI = jest.spyOn(dbiService, 'postToDBI');
      await assetsService.postAssets(ASSETS_POST);
      expect(postAssetsToDBI).toHaveBeenCalledWith(DBI_URIs.UI_Config_Assets, ASSETS_POST);
    });

    it('should return assets after post', async () => {
      jest.spyOn(dbiService, 'postToDBI').mockResolvedValue(ASSETS_RESPONSE);
      expect(await assetsService.postAssets(ASSETS_POST)).toMatchObject(ASSETS_RESPONSE);
    });
  });
});
