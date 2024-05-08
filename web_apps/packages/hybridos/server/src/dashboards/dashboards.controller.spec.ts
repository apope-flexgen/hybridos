import { Test, TestingModule } from '@nestjs/testing';
import { DashboardsController } from './dashboards.controller';
import { DBI_SERVICE } from '../dbi/dbi.interface';
import { DASHBOARDS_SERVICE } from './dashboards.constants';
import { IDashboardsService } from './dashboards.interface';
import { BadRequestException } from '@nestjs/common';
import { DashboardCard } from './dtos/dashboard.dto';
import { UI_CONFIG_DASH, UI_CONFIG_DASH_AFTER_POST, POST_DASH } from './dashboards.mocks';

describe('DashboardsController', () => {
  let dashboardsController: DashboardsController;
  let dashboardsService: IDashboardsService;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      controllers: [DashboardsController],
      providers: [
        {
          provide: DBI_SERVICE,
          useValue: {
            getFromDBI: jest.fn(),
            postToDBI: jest.fn(),
          },
        },
        {
          provide: DASHBOARDS_SERVICE,
          useValue: {
            getDashboards: jest.fn(),
            postDashboards: jest.fn(),
          },
        },
      ],
    }).compile();

    dashboardsController = module.get<DashboardsController>(DashboardsController);
    dashboardsService = module.get<IDashboardsService>(DASHBOARDS_SERVICE);
  });

  it('dashboardsService should be defined', () => {
    expect(dashboardsService).toBeDefined();
  });

  describe('test getDashboards', () => {
    it('should return dashboards', async () => {
      jest.spyOn(dashboardsService, 'getDashboards').mockResolvedValue(UI_CONFIG_DASH);
      expect(await dashboardsController.getDashboards()).toMatchObject(UI_CONFIG_DASH);
    });
  });

  describe('test postDashboards', () => {
    it('should use the data body', async () => {
      const postDashService = jest.spyOn(dashboardsService, 'postDashboards');
      await dashboardsController.postDashboards(POST_DASH);
      expect(postDashService).toHaveBeenCalledWith(POST_DASH);
    });

    it('should return dashboards after post', async () => {
      jest.spyOn(dashboardsService, 'postDashboards').mockResolvedValue(UI_CONFIG_DASH_AFTER_POST);
      expect(await dashboardsController.postDashboards(POST_DASH)).toMatchObject(
        UI_CONFIG_DASH_AFTER_POST,
      );
    });

    it('should throw an exception', async () => {
      jest
        .spyOn(dashboardsService, 'postDashboards')
        .mockRejectedValue(new BadRequestException('error'));
      await expect(
        dashboardsController.postDashboards('bad' as unknown as DashboardCard),
      ).rejects.toThrow();
    });
  });
});
