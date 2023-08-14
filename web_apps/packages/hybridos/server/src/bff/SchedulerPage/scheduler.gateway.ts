import { SubscribeMessage, WebSocketGateway, WebSocketServer } from '@nestjs/websockets';
import { Observable } from 'rxjs';
import { SocketMessageBody } from 'src/decorators/socketMessageBody.decorator';
import { Server } from 'ws';
import { MergedSchedulerData } from './scheduler.interface';
import { SchedulerService } from './scheduler.service';
import { UseWsFilters } from '../../decorators/ws.filters.decorator';
import { UseWsInterceptors } from '../../decorators/ws.interceptors.decorator';

@WebSocketGateway({
  cors: {
    origin: '*',
  },
})
@UseWsFilters()
export class SchedulerGateway {
  constructor(private readonly schedulerService: SchedulerService) {}

  @WebSocketServer()
  server: Server;

  @SubscribeMessage('scheduler')
  @UseWsInterceptors()
  scheduler(@SocketMessageBody() URIs: string[]): Observable<MergedSchedulerData> {
    const schedulerDataStream = this.schedulerService.getMergedStream(URIs);

    return schedulerDataStream;
  }
}
