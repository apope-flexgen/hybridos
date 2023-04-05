import { SubscribeMessage, WebSocketGateway, WebSocketServer } from '@nestjs/websockets'
import { Observable, tap } from 'rxjs'
import { Server } from 'ws'
import { SiteStatusResponse } from './sitestatus.interface'
import { SiteStatusService } from './sitestatus.service'

@WebSocketGateway({
    cors: {
        origin: '*',
    },
})
export class SiteStatusGateway {
    constructor(private readonly siteStatusService: SiteStatusService) {}

    @WebSocketServer()
    server: Server

    @SubscribeMessage('sitestatus')
    async siteStatus(): Promise<Observable<SiteStatusResponse>> {
        const subscribeObservable = await this.siteStatusService.subscribeToSiteStatus()

        return subscribeObservable
    }
}
