import { Inject, Req, UseGuards, UsePipes, ValidationPipe } from '@nestjs/common';
import { Throttle } from '@nestjs/throttler';
import { SubscribeMessage, WebSocketGateway, WebSocketServer } from '@nestjs/websockets';
import { Observable } from 'rxjs';
import { FIMS_WS_LIMIT, FIMS_WS_TTL } from '../environment/appEnv.constants';
import { Server } from 'ws';

import { FimsMsgDTO } from './dto/fims.dto';
import { FimsWebSocketGuard } from './guards/fims.ws.guard';
import { FIMS_SERVICE, FimsMsg, IFimsService } from './interfaces/fims.interface';
import { WsThrottleExceptionFilter } from './wsthrottler.filter';
import { WsThrottlerGuard } from './wsthrottler.guard';
import { SocketMessageBody } from '../decorators/socketMessageBody.decorator';
import { UseWsFilters } from '../decorators/ws.filters.decorator';
import { UseWsInterceptors } from '../decorators/ws.interceptors.decorator';

@WebSocketGateway({
  cors: {
    origin: '*',
  },
})
@UseGuards(WsThrottlerGuard, FimsWebSocketGuard)
@Throttle(FIMS_WS_LIMIT, FIMS_WS_TTL)
@UseWsFilters(new WsThrottleExceptionFilter())
export class FimsGateway {
  constructor(@Inject(FIMS_SERVICE) private readonly fimsService: IFimsService) {}

  @WebSocketServer()
  server: Server;

  @SubscribeMessage('request')
  @UsePipes(new ValidationPipe({ transform: true }))
  async request(@SocketMessageBody() data: FimsMsgDTO): Promise<FimsMsg> {
    try {
      const msg: FimsMsg = {
        method: data.method,
        uri: data.uri,
        replyto: data.replyto,
        body: data.body,
        username: data.username,
      };

      return this.fimsService.send(msg);
    } catch (e) {
      console.log('request failed: ', e);
    }
  }

  @SubscribeMessage('fimsNoReply')
  @UsePipes(new ValidationPipe({ transform: true }))
  async fimsNoReply(@SocketMessageBody() data: FimsMsgDTO): Promise<void> {
    try {
      const msg: FimsMsg = {
        method: data.method,
        uri: data.uri,
        replyto: data.replyto,
        body: data.body,
        username: data.username,
      };

      this.fimsService.send(msg);
    } catch (e) {
      console.log('request failed: ', e);
    }
  }

  @SubscribeMessage('subscribe')
  @UseWsInterceptors()
  @UsePipes(new ValidationPipe({ transform: true }))
  subscribe(@SocketMessageBody() uri: string, @Req() req: any): Observable<FimsMsg> {
    return this.fimsService.subscribe(uri, req);
  }
}
