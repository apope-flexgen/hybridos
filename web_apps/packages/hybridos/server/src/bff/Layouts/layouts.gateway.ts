import { SubscribeMessage, WebSocketGateway, WebSocketServer } from '@nestjs/websockets';
import { Observable } from 'rxjs';
import { Server, WebSocket } from 'ws';
import { LayoutsService } from './layouts.service';
import { LayoutsResponse } from './responses';
import { UseWsFilters } from '../../decorators/ws.filters.decorator';
import { UseWsInterceptors } from '../../decorators/ws.interceptors.decorator';

@WebSocketGateway({
  cors: {
    origin: '*',
  },
})
@UseWsFilters()
export class LayoutsGateway {
  private clients: WebSocket[] = [];
  constructor(private readonly layoutsService: LayoutsService) {}

  @WebSocketServer()
  server: Server;

  handleConnection(client: WebSocket, ...args: any[]) {
    this.clients.push(client);
  }

  handleDisconnect(client: WebSocket) {
    this.clients = this.clients.filter((c) => c !== client);
  }

  @UseWsInterceptors()
  @SubscribeMessage('layouts')
  async layouts(): Promise<Observable<LayoutsResponse>> {
    const subscribeObservable = await this.layoutsService.getLayouts();
    return subscribeObservable;
  }

  async sendToAllClients(): Promise<void> {
    const data = await this.layoutsService.getLayouts();
    for (const client of this.clients) {
      if (client.readyState === WebSocket.OPEN) {
        client.send(JSON.stringify({ namespace: 'layouts', data }));
      }
    }
  }
}
