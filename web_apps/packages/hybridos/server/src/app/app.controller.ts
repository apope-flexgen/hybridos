import { Controller, Get } from '@nestjs/common'
import { IAppService, APP_SERVICE } from './app.service.interface'
import { Inject } from '@nestjs/common'
import { ApiOkResponse, ApiTags } from '@nestjs/swagger';
import { SiteConfigurationResponse } from './responses/siteConfiguration.response';
import { Public } from 'src/decorators/public.decorator';

@ApiTags('app')
@Controller()
export class AppController {
    constructor(
        @Inject(APP_SERVICE)
        private readonly appService: IAppService
    ) { }
    @ApiOkResponse({ type: SiteConfigurationResponse })
    @Get('/app')
    async appInit() {
        return this.appService.getSiteConfiguration()
    }
    @Public()
    @Get('/login-info')
    async loginInfo() {
        return this.appService.getLoginInfo()
    }
}
