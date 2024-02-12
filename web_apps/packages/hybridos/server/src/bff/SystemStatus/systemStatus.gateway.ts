import { SubscribeMessage, WebSocketGateway, WebSocketServer } from '@nestjs/websockets';
import { Observable } from 'rxjs';
import { Server } from 'ws';
import { SystemStatusService } from './systemStatus.service';
import { UseWsFilters } from '../../decorators/ws.filters.decorator';
import { UseWsInterceptors } from '../../decorators/ws.interceptors.decorator';
import { ServiceStatusResponse } from './dto/serviceStatusResponse.dto';
import { SystemStatusPruningInterceptor } from 'src/bff/SystemStatus/interceptors/systemStatus.pruning.interceptor';

@WebSocketGateway({
  cors: {
    origin: '*',
  },
})
@UseWsFilters()
export class SystemStatusGateway {
  constructor(private readonly systemStatusService: SystemStatusService) {}

  @WebSocketServer()
  server: Server;

  @SubscribeMessage('systemStatus')
  @UseWsInterceptors(SystemStatusPruningInterceptor)
  systemStatus(): Observable<ServiceStatusResponse> {
    const systemStatusDataStream = this.systemStatusService.subscribeToSystemStatus();

    return systemStatusDataStream;
  }
}
