/* eslint-disable max-nested-callbacks */
import { Test, TestingModule } from '@nestjs/testing';
import { IDashboardsService } from './dashboards.interface';
import { DashboardsService } from './dashboards.service';
import { DBI_SERVICE, DBI_URIs, IDBIService } from '../dbi/dbi.interface';
import { UI_CONFIG_DASH, UI_CONFIG_DASH_AFTER_POST, POST_DASH } from './dashboards.mocks';

describe('DashboardsService', () => {
  let dashboardsService: IDashboardsService;
  let dbiService: IDBIService;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [
        DashboardsService,
        {
          provide: DBI_SERVICE,
          useValue: {
            getFromDBI: jest.fn(),
            postToDBI: jest.fn(),
          },
        },
      ],
    }).compile();

    dashboardsService = module.get<DashboardsService>(DashboardsService);
    dbiService = module.get<IDBIService>(DBI_SERVICE);
  });

  it('dbiService should be defined', () => {
    expect(dbiService).toBeDefined();
  });

  describe('test getDashboards', () => {
    it('should return dashboards', async () => {
      jest.spyOn(dbiService, 'getFromDBI').mockResolvedValue(UI_CONFIG_DASH);
      expect(await dashboardsService.getDashboards()).toMatchObject(UI_CONFIG_DASH);
    });
  });

  describe('test postDashboards', () => {
    it('should use the data body', async () => {
      const postDashToDBI = jest.spyOn(dbiService, 'postToDBI');
      await dashboardsService.postDashboards(POST_DASH);
      expect(postDashToDBI).toHaveBeenCalledWith(DBI_URIs.UI_Config_Dashboard, POST_DASH);
    });

    it('should return dashboards after post', async () => {
      jest.spyOn(dbiService, 'postToDBI').mockResolvedValue(UI_CONFIG_DASH_AFTER_POST);
      expect(await dashboardsService.postDashboards(POST_DASH)).toMatchObject(
        UI_CONFIG_DASH_AFTER_POST,
      );
    });
  });
});
