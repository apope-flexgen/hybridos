import { MessageBody,SubscribeMessage, WebSocketGateway, WebSocketServer } from '@nestjs/websockets'
import { Observable, tap } from 'rxjs'
import { Server } from 'ws'
import { MergedSchedulerData } from './scheduler.interface'
import { SchedulerService } from './scheduler.service'

@WebSocketGateway({
    cors: {
        origin: '*',
    },
})
export class SchedulerGateway {
    constructor(private readonly schedulerService: SchedulerService) {}
    @WebSocketServer()
    server: Server
    @SubscribeMessage('scheduler')
    async scheduler(
        @MessageBody() URIs: string[]
    ): Promise<Observable<MergedSchedulerData>> {
        const schedulerDataStream = await this.schedulerService.getMergedStream(URIs)

        return schedulerDataStream
    }
}
