import { SubscribeMessage, WebSocketGateway, WebSocketServer } from '@nestjs/websockets';
import { Observable } from 'rxjs';
import { Server } from 'ws';
import { SiteStatusResponse } from './sitestatus.interface';
import { SiteStatusService } from './sitestatus.service';
import { UseWsFilters } from '../../decorators/ws.filters.decorator';
import { UseWsInterceptors } from '../../decorators/ws.interceptors.decorator';

@WebSocketGateway({
  cors: {
    origin: '*',
  },
})
@UseWsFilters()
export class SiteStatusGateway {
  constructor(private readonly siteStatusService: SiteStatusService) {}

  @WebSocketServer()
  server: Server;

  @UseWsInterceptors()
  @SubscribeMessage('sitestatus')
  async siteStatus(): Promise<Observable<SiteStatusResponse>> {
    const subscribeObservable = await this.siteStatusService.subscribeToSiteStatus();

    return subscribeObservable;
  }
}
