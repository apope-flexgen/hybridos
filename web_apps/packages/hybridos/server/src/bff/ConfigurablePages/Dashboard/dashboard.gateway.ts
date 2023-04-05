import { SubscribeMessage, WebSocketGateway, WebSocketServer } from '@nestjs/websockets'
import { Observable } from 'rxjs'
import { Server } from 'ws'
import { ConfigurablePageDTO } from 'shared/types/dtos/configurablePages.dto'
import { DashboardService } from './dashboard.service'

@WebSocketGateway({
    cors: {
        origin: '*',
    },
})
export class DashboardGateway {
    constructor(private readonly dashboardService: DashboardService) {}
    @WebSocketServer()
    server: Server
    @SubscribeMessage('dashboard')
    async dashboard(): Promise<
        Observable<ConfigurablePageDTO>
    > {
        const subscribeObservable = await this.dashboardService.subscribeToDashboard()

        return subscribeObservable
    }
}
