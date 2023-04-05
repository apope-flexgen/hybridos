import { Inject, Req, UseFilters, UseGuards, UsePipes, ValidationPipe } from '@nestjs/common'
import { Throttle } from '@nestjs/throttler'
import {
    MessageBody,
    SubscribeMessage,
    WebSocketGateway,
    WebSocketServer,
} from '@nestjs/websockets'
import { Observable } from 'rxjs'
import { Server, WebSocket } from 'ws'

import { FimsMsgDTO } from './dto/fims.dto'
import { FimsWebSocketGuard } from './guards/fims.ws.guard'
import { FIMS_SERVICE, FimsMsg, IFimsService } from './interfaces/fims.interface'
import { WsThrottleExceptionFilter } from './wsthrottler.filter'
import { WsThrottlerGuard } from './wsthrottler.guard'

// FIXME: these should probably come from a config/db.
// also, these values are not clearly defined in the old node server,
// because it uses a different library.
const WS_TTL = 10000
const WS_LIMIT = 10000

@WebSocketGateway({
    cors: {
        origin: '*',
    },
})
@UseGuards(WsThrottlerGuard, FimsWebSocketGuard)
@Throttle(WS_LIMIT, WS_TTL)
@UseFilters(new WsThrottleExceptionFilter())
export class FimsGateway {
    constructor(@Inject(FIMS_SERVICE) private readonly fimsService: IFimsService) {}
    @WebSocketServer()
    server: Server
    @SubscribeMessage('request')
    @UsePipes(new ValidationPipe({ transform: true }))
    async request(@MessageBody() data: FimsMsgDTO): Promise<FimsMsg> {
        try {
            const msg: FimsMsg = {
                method: data.method,
                uri: data.uri,
                replyto: data.replyto,
                body: data.body,
                username: data.username,
            }

            return this.fimsService.send(msg)
        } catch (e) {
            console.log('request failed: ', e)
        }
    }
    @SubscribeMessage('fimsNoReply')
    @UsePipes(new ValidationPipe({ transform: true }))
    async fimsNoReply(@MessageBody() data: FimsMsgDTO): Promise<void> {
        try {
            const msg: FimsMsg = {
                method: data.method,
                uri: data.uri,
                replyto: data.replyto,
                body: data.body,
                username: data.username,
            }

            this.fimsService.send(msg)
        } catch (e) {
            console.log('request failed: ', e)
        }
    }
    @SubscribeMessage('subscribe')
    @UsePipes(new ValidationPipe({ transform: true }))
    subscribe(@MessageBody() uri: string, @Req() req: any): Observable<FimsMsg> {
        console.log('req: ', typeof req, req)
        return this.fimsService.subscribe(uri, req)
    }
}
