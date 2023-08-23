import { Test, TestingModule } from '@nestjs/testing';
import { AssetsController } from './assets.controller';
import { IAssetsService } from './assets.interface';
import { ASSETS_SERVICE } from './assets.constants';
import { LAYOUTS_SERVICE } from '../bff/Layouts/layouts.constants';
import { LayoutsGateway } from '../bff/Layouts/layouts.gateway';
import { Assets } from './dtos/asset.dto';
import { LoggingService } from '../logging/logging.service';
import { LOGGING_SERVICE } from '../logging/logging.constants';
import { ILayoutsService } from '../bff/Layouts/layouts.interface';
import { ASSETS_RESPONSE, ASSETS_POST, ASSETS_REDUCED, ASSETS_REDUCED_DUPLICATE, ASSETS_POST_DUPLICATE } from './assets.mocks';

describe('AssetsController', () => {
    let assetsController: AssetsController;
    let assetsService: IAssetsService;
    let loggingService: LoggingService;
    let layoutsService: ILayoutsService;
    let layoutsGateway: LayoutsGateway;

    beforeEach(async () => {
        const module: TestingModule = await Test.createTestingModule({
            controllers: [AssetsController],
            providers: [
                {
                    provide: LOGGING_SERVICE,
                    useValue: {
                        error: jest.fn(),
                    },
                },
                {
                    provide: ASSETS_SERVICE,
                    useValue: {
                        getAssets: jest.fn(),
                        postAssets: jest.fn(),
                    },
                },
                {
                    provide: LAYOUTS_SERVICE,
                    useValue: {
                        getLayouts: jest.fn(),
                        postLayouts: jest.fn(),
                    },
                },
                {
                    provide: LayoutsGateway,
                    useValue: {
                        sendToAllClients: jest.fn(),
                    },
                },
            ],
        }).compile();

        assetsController = module.get<AssetsController>(AssetsController);
        assetsService = module.get<IAssetsService>(ASSETS_SERVICE);
        loggingService = module.get<LoggingService>(LOGGING_SERVICE);
        layoutsService = module.get<ILayoutsService>(LAYOUTS_SERVICE);
        layoutsGateway = module.get<LayoutsGateway>(LayoutsGateway);
    });

    describe('test getAssets', () => {
        it('should return assets', async () => {
            jest
                .spyOn(assetsService, 'getAssets')
                .mockResolvedValue(ASSETS_RESPONSE);
            expect(await assetsController.getAssets()).toMatchObject(ASSETS_RESPONSE);
        });

        describe('test error throw', () => {
            beforeEach(async () => {
                jest
                    .spyOn(assetsService, 'getAssets')
                    .mockRejectedValue(new Error('example error'));
            });

            it('should return empty string', async () => {
                expect(await assetsController.getAssets()).toEqual('');
            });

            it('should log error', async () => {
                const loggingMock = jest.spyOn(loggingService, 'error');
                await assetsController.getAssets();
                expect(loggingMock).toHaveBeenCalled();
            });
        })
    });

    describe('tests postAssets', () => {
        it('should use the data body', async () => {
            const postAssetsService = jest.spyOn(assetsService, 'postAssets');
            await assetsController.postAssets(ASSETS_POST);
            expect(postAssetsService).toHaveBeenCalledWith(ASSETS_POST);
        });

        it('should return data body', async () => {
            jest
                .spyOn(assetsService, 'postAssets')
                .mockResolvedValue(ASSETS_RESPONSE);
            expect(await assetsController.postAssets(ASSETS_POST)).toMatchObject(ASSETS_RESPONSE);
        });

        it('should reduce assets and postLayouts', async () => {
            const postLayoutsService = jest.spyOn(layoutsService, 'postLayouts');
            await assetsController.postAssets(ASSETS_POST);
            expect(postLayoutsService).toHaveBeenCalledWith(ASSETS_REDUCED);
        });

        it('should sendToAllClients', async () => {
            const sendToAllClientsGateway = jest.spyOn(layoutsGateway, 'sendToAllClients');
            await assetsController.postAssets(ASSETS_POST);
            expect(sendToAllClientsGateway).toHaveBeenCalled();
        });

        it('should handle duplicate key names', async() => {
            const postLayoutsService = jest.spyOn(layoutsService, 'postLayouts');
            await assetsController.postAssets(ASSETS_POST_DUPLICATE);
            expect(postLayoutsService).toHaveBeenCalledWith(ASSETS_REDUCED_DUPLICATE);
        });
        
        it('should throw an exception', async () => {
            await expect(assetsController.postAssets('bad input' as unknown as Assets))
                .rejects.toThrow();
        });
    });
});