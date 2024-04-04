import { SubscribeMessage, WebSocketGateway, WebSocketServer } from '@nestjs/websockets';
import { Observable } from 'rxjs';
import { Server } from 'ws';
import { AlertsService } from './alerts.service';
import { UseWsFilters } from '../../decorators/ws.filters.decorator';
import { UseWsInterceptors } from '../../decorators/ws.interceptors.decorator';

@WebSocketGateway({
  cors: {
    origin: '*',
  },
})
@UseWsFilters()
export class AlertsGateway {
  constructor(private readonly alertsService: AlertsService) {}

  @WebSocketServer()
  server: Server;

  @SubscribeMessage('alerts')
  @UseWsInterceptors()
  alerts(): Observable<any> {
    const alertsDataStream = this.alertsService.getAlertingObservable();

    return alertsDataStream;
  }
}
