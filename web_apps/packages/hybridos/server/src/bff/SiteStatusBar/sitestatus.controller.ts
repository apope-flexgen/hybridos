import { Controller, Inject, Sse } from '@nestjs/common';
import { ApiDefaultResponse, ApiOkResponse, ApiSecurity, ApiTags } from '@nestjs/swagger';
import { Observable } from 'rxjs';
import { SiteStatusResponse } from './responses/siteStatus.response';
import { DefaultApiError } from 'src/exceptions/defaultResponse.exception';
import {
  ISiteStatusService,
  SITE_STATUS_SERVICE,
} from 'src/bff/SiteStatusBar/sitestatus.interface';

@ApiSecurity('bearerAuth')
@ApiDefaultResponse({ type: DefaultApiError })
@ApiTags('sitestatus')
@ApiOkResponse({ type: SiteStatusResponse })
@Controller()
export class SiteStatusController {
  constructor(
    @Inject(SITE_STATUS_SERVICE)
    private readonly siteStatusService: ISiteStatusService,
  ) {}
  @Sse('sitestatus')
  @ApiOkResponse({ type: SiteStatusResponse })
  getSiteStatus(): Promise<Observable<SiteStatusResponse>> {
    return this.siteStatusService.subscribeToSiteStatus();
  }
}
