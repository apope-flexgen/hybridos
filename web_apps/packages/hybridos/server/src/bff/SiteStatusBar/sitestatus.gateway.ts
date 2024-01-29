import { SubscribeMessage, WebSocketGateway, WebSocketServer } from '@nestjs/websockets';
import { Observable } from 'rxjs';
import { Server } from 'ws';
import {
  ISiteStatusService,
  SITE_STATUS_SERVICE,
  SiteStatusResponse,
} from './sitestatus.interface';
import { UseWsFilters } from '../../decorators/ws.filters.decorator';
import { UseWsInterceptors } from '../../decorators/ws.interceptors.decorator';
import { Inject } from '@nestjs/common';

@WebSocketGateway({
  cors: {
    origin: '*',
  },
})
@UseWsFilters()
export class SiteStatusGateway {
  constructor(
    @Inject(SITE_STATUS_SERVICE)
    private readonly siteStatusService: ISiteStatusService,
  ) {}

  @WebSocketServer()
  server: Server;

  @UseWsInterceptors()
  @SubscribeMessage('sitestatus')
  async siteStatus(): Promise<Observable<SiteStatusResponse>> {
    const subscribeObservable = await this.siteStatusService.subscribeToSiteStatus();

    return subscribeObservable;
  }
}
