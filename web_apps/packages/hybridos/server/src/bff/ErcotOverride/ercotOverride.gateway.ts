import { SubscribeMessage, WebSocketGateway, WebSocketServer } from '@nestjs/websockets';
import { SocketMessageBody } from 'src/decorators/socketMessageBody.decorator';
import { Observable } from 'rxjs';
import { Server } from 'ws';
import { ErcotOverrideDto } from './ercotOverride.interface';
import { ErcotOverrideService } from './ercotOverride.service';
import { UseWsFilters } from '../../decorators/ws.filters.decorator';
import { UseWsInterceptors } from '../../decorators/ws.interceptors.decorator';

@WebSocketGateway({
  cors: {
    origin: '*',
  },
})
@UseWsFilters()
export class ErcotOverrideGateway {
  constructor(private readonly ercotOverrideService: ErcotOverrideService) {}

  @WebSocketServer()
  server: Server;

  @SubscribeMessage('ercotOverride')
  @UseWsInterceptors()
  ercotOverride(@SocketMessageBody() siteId: string): Observable<ErcotOverrideDto> {
    const ercotOverrideDataStream = this.ercotOverrideService.getUriSpecificObservable(siteId);

    return ercotOverrideDataStream;
  }
}
