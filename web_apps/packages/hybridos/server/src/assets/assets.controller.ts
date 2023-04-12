import { Body, Controller, Get, Inject, Post, Req } from '@nestjs/common'
import { ApiOkResponse, ApiTags } from '@nestjs/swagger';
import { LAYOUTS_SERVICE } from 'src/layouts/layouts.constants';
import { ILayoutsService } from 'src/layouts/layouts.interface';
import { ASSETS_SERVICE } from './assets.constants';
import { AssetsResponse } from './responses';
import { IAssetsService } from './assets.interface';
import { Layout } from '../../../shared/types/dtos/layouts.dto'
import { UserFromAccessToken } from 'src/decorators/userFromAccessToken.decorator';
import { User } from 'src/users/dtos/user.dto';

@ApiTags('assets')
@Controller('assets')
export class AssetsController {
    constructor(
        @Inject(ASSETS_SERVICE)
        private readonly assetsService: IAssetsService,
        @Inject(LAYOUTS_SERVICE)
        private readonly layoutsService: ILayoutsService
    ) { }
    @ApiOkResponse({ type: AssetsResponse })
    @Get()
    async getAssets() {
        return this.assetsService.getAssets()
    }
    @ApiOkResponse({ type: AssetsResponse })
    @Post()
    async postAssets(@Body() assets, @UserFromAccessToken() user: User): Promise<AssetsResponse> {
        const data: Layout[] = assets.data.reduce((acc, el, index) => {
            let key = el.info.assetKey || el.info.name.toLowerCase().replaceAll(' ', '_');
            if (acc.some((item) => item.info.key === key)) {
                key = `${key}_${index}`
            }
            el.info.assetKey = key;
            acc.push({
                info: {
                    name: el.info.name,
                    key
                }
            })
            return acc
        }, [])
        
        await this.layoutsService.postLayouts({ data }, user)
        return this.assetsService.postAssets(assets, user) 
    }
}
