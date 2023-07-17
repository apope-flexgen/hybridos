import { SubscribeMessage, WebSocketGateway, WebSocketServer } from '@nestjs/websockets';
import { Observable } from 'rxjs';
import { Server } from 'ws';
import { ConfigurablePageDTO } from 'shared/types/dtos/configurablePages.dto';
import { DashboardService } from './dashboard.service';
import { SocketNamespaceInterceptor } from 'src/interceptors/socketNamespace.interceptor';
import { UseInterceptors } from '@nestjs/common';
import { UseFilters } from '@nestjs/common';
import { AppExceptionsFilter } from 'src/filters/all-exceptions.filter';
import { DashboardLayoutInterceptor } from './tableDashboard.interceptor';

@WebSocketGateway({
  cors: {
    origin: '*',
  },
})
@UseFilters(AppExceptionsFilter)
@UseInterceptors(SocketNamespaceInterceptor, DashboardLayoutInterceptor)
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
