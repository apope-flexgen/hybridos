import { Body, Controller, Get, Inject, Post, Req } from '@nestjs/common';
import { ApiDefaultResponse, ApiOkResponse, ApiSecurity, ApiTags } from '@nestjs/swagger';
import { LAYOUTS_SERVICE } from '../bff/Layouts/layouts.constants';
import { ILayoutsService } from '../bff/Layouts/layouts.interface';
import { ASSETS_SERVICE } from './assets.constants';
import { AssetsResponse } from './responses';
import { IAssetsService } from './assets.interface';
import { Layout } from '../../../shared/types/dtos/layouts.dto';
import { Assets } from './dtos/asset.dto';
import { DefaultApiError } from '../exceptions/defaultResponse.exception';
import { LayoutsGateway } from '../bff/Layouts/layouts.gateway';
import { LogText } from '../logging/log_text/log-text';
import { LoggingService } from '../logging/logging.service';
import { LOGGING_SERVICE } from '../logging/logging.constants';

@ApiTags('assets')
@ApiDefaultResponse({ type: DefaultApiError })
@Controller('assets')
@ApiSecurity('bearerAuth')
export class AssetsController {
  constructor(
    @Inject(LOGGING_SERVICE)
    private readonly loggingService: LoggingService,
    @Inject(ASSETS_SERVICE)
    private readonly assetsService: IAssetsService,
    @Inject(LAYOUTS_SERVICE)
    private readonly layoutsService: ILayoutsService,
    @Inject(LayoutsGateway)
    private readonly layoutsGateway: LayoutsGateway,
  ) {}
  @ApiOkResponse({ type: AssetsResponse })
  @Get()
  async getAssets() {
    try {
      return await this.assetsService.getAssets();
    } catch (e) {
      const log: LogText = {
        message: e.message,
        stack: e.stack,
        url: 'assets',
      };
      this.loggingService.error(log, '');
      return '';
    }
  }
  @ApiOkResponse({ type: AssetsResponse })
  @Post()
  async postAssets(@Body() assets: Assets): Promise<AssetsResponse> {
    const data: Layout[] = assets.data.reduce((acc, el, index) => {
      let key = el.info.name.toLowerCase().replaceAll(' ', '_');
      if (!key.includes('_tab')) {
        key = `${key}_tab`;
      }
      if (acc.some((item) => item.info.key === key)) {
        key = `${key}_${index + 1}`;
      }
      el.info.assetKey = key;
      acc.push({
        info: {
          name: el.info.name,
          key,
        },
      });
      return acc;
    }, []);

    await this.layoutsService.postLayouts({ data });
    await this.layoutsGateway.sendToAllClients();
    return this.assetsService.postAssets(assets);
  }
}
