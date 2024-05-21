import { SubscribeMessage, WebSocketGateway, WebSocketServer } from '@nestjs/websockets';
import { Observable } from 'rxjs';
import { Server } from 'ws';
import { ConfigurablePageDTO } from 'shared/types/dtos/configurablePages.dto';
import { UseWsFilters } from '../../../decorators/ws.filters.decorator';
import { SiteDiagramConfigNotFoundExceptionsFilter } from './filters/siteDiagram.filters';
import { UseWsInterceptors } from 'src/decorators/ws.interceptors.decorator';
import { SiteDiagramPruningInterceptor } from './interceptors/siteDiagram.pruning.interceptor';
import { ISiteDiagramService, SITE_DIAGRAM_SERVICE } from './siteDiagram.interface';
import { Inject } from '@nestjs/common';
import { SocketMessageBody } from 'src/decorators/socketMessageBody.decorator';

@WebSocketGateway({
  cors: {
    origin: '*',
  },
})
@UseWsFilters(SiteDiagramConfigNotFoundExceptionsFilter)
@UseWsInterceptors(SiteDiagramPruningInterceptor)
export class SiteDiagramGateway {
  constructor(
    @Inject(SITE_DIAGRAM_SERVICE)
    private readonly siteDiagramService: ISiteDiagramService,
  ) {}
  @WebSocketServer()
  server: Server;
  @SubscribeMessage('siteDiagram')
  async siteDiagram(
    @SocketMessageBody() siteId?: string,
  ): Promise<Observable<ConfigurablePageDTO>> {
    return this.siteDiagramService.subscribeToSiteDiagram(siteId);
  }
}
