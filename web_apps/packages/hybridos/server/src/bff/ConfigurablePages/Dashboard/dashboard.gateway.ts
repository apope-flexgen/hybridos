import { SubscribeMessage, WebSocketGateway, WebSocketServer } from '@nestjs/websockets';
import { Observable } from 'rxjs';
import { Server } from 'ws';
import { ConfigurablePageDTO } from 'shared/types/dtos/configurablePages.dto';
import { DashboardService } from './dashboard.service';
import { DashboardLayoutInterceptor } from './interceptors/dashboard.layout.interceptor';
import { UseWsFilters } from '../../../decorators/ws.filters.decorator';
import { DashboardConfigNotFoundExceptionsFilter } from './filters/dashboard.filters';
import { UseWsInterceptors } from '../../../decorators/ws.interceptors.decorator';
import { DashboardPruningInterceptor } from 'src/bff/ConfigurablePages/Dashboard/interceptors/dashboard.pruning.interceptor';

@WebSocketGateway({
  cors: {
    origin: '*',
  },
})
@UseWsFilters(DashboardConfigNotFoundExceptionsFilter)
@UseWsInterceptors(DashboardLayoutInterceptor, DashboardPruningInterceptor)
export class DashboardGateway {
  constructor(private readonly dashboardService: DashboardService) {}
  @WebSocketServer()
  server: Server;
  @SubscribeMessage('dashboard')
  async dashboard(): Promise<Observable<ConfigurablePageDTO>> {
    const subscribeObservable = await this.dashboardService.subscribeToDashboard();

    return subscribeObservable;
  }
}
