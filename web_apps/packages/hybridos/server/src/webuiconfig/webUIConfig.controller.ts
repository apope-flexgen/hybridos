import { Controller, Get } from '@nestjs/common'
import {
    IWebUIConfigService,
    UI_CONFIG_SERVICE,
} from './webUIConfig.service.interface'
import { Inject } from '@nestjs/common'
import {
    ApiOkResponse,
    ApiDefaultResponse,
    ApiSecurity,
    ApiTags,
} from '@nestjs/swagger'
import { SiteConfigurationResponse } from './responses/siteConfiguration.response'
import { Public } from 'src/decorators/public.decorator'
import { LoginInfoResponse } from './responses/product.response'
import { DefaultApiError } from 'src/exceptions/defaultResponse.exception'

@ApiTags('web-ui-config')
@ApiDefaultResponse({ type: DefaultApiError })
@Controller()
export class WebUIConfigController {
    constructor(
        @Inject(UI_CONFIG_SERVICE)
        private readonly appService: IWebUIConfigService
    ) {}
    @ApiOkResponse({ type: SiteConfigurationResponse })
    @Get('/web-ui-config')
    @ApiSecurity('bearerAuth')
    async appInit(): Promise<SiteConfigurationResponse> {
        return this.appService.getSiteConfiguration()
    }
    @Public()
    @ApiOkResponse({ type: LoginInfoResponse })
    @Get('/login-info')
    async loginInfo(): Promise<LoginInfoResponse> {
        return this.appService.getLoginInfo()
    }
}
