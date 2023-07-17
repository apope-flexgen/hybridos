import { Controller, Sse } from '@nestjs/common'
import { ApiDefaultResponse, ApiOkResponse, ApiSecurity, ApiTags } from '@nestjs/swagger'
import { Observable } from 'rxjs'
import { SiteStatusResponse } from './responses/siteStatus.response'
import { SiteStatusService } from './sitestatus.service'
import { DefaultApiError } from 'src/exceptions/defaultResponse.exception'

@ApiSecurity('bearerAuth')
@ApiDefaultResponse({ type: DefaultApiError })

@ApiTags('sitestatus')
@ApiOkResponse({type: SiteStatusResponse})
@Controller()
export class SiteStatusController {
    constructor(private readonly siteStatusService: SiteStatusService) {}
    @Sse('sitestatus')
    @ApiOkResponse({ type: SiteStatusResponse })
    getSiteStatus(): Promise<Observable<SiteStatusResponse>> {
        return this.siteStatusService.subscribeToSiteStatus()
    }
}
