/* eslint-disable */
import { Controller, Sse } from '@nestjs/common'
import { tap } from 'rxjs'
import { SiteStatusService } from './sitestatus.service'

@Controller()
export class SiteStatusController {
    constructor(private readonly siteStatusService: SiteStatusService) {}

    @Sse('sitestatus')
    getSiteStatus() {
        return this.siteStatusService.subscribeToSiteStatus()
    }
}
