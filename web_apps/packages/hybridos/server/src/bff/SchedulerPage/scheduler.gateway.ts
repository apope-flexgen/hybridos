import { UseInterceptors } from '@nestjs/common'
import {
    SubscribeMessage,
    WebSocketGateway,
    WebSocketServer,
} from '@nestjs/websockets'
import { Observable } from 'rxjs'
import { SocketMessageBody } from 'src/decorators/socketMessageBody.decorator'
import { SocketNamespaceInterceptor } from 'src/interceptors/socketNamespace.interceptor'
import { UseFilters } from '@nestjs/common'
import { AppExceptionsFilter } from 'src/filters/all-exceptions.filter'
import { Server } from 'ws'
import { MergedSchedulerData } from './scheduler.interface'
import { SchedulerService } from './scheduler.service'

@WebSocketGateway({
    cors: {
        origin: '*',
    },
})
@UseFilters(AppExceptionsFilter)
export class SchedulerGateway {
    constructor(private readonly schedulerService: SchedulerService) {}

    @WebSocketServer()
    server: Server

    @SubscribeMessage('scheduler')
    @UseInterceptors(SocketNamespaceInterceptor)
    scheduler(
        @SocketMessageBody() URIs: string[]
    ): Observable<MergedSchedulerData> {
        const schedulerDataStream = this.schedulerService.getMergedStream(URIs)

        return schedulerDataStream
    }
}
