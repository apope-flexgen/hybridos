import { UseInterceptors } from '@nestjs/common'
import { SubscribeMessage, WebSocketGateway, WebSocketServer } from '@nestjs/websockets'
import { Observable } from 'rxjs'
import { SocketNamespaceInterceptor } from 'src/interceptors/socketNamespace.interceptor'
import { UseFilters } from '@nestjs/common'
import { AppExceptionsFilter } from 'src/filters/all-exceptions.filter'
import { Server } from 'ws'
import { SiteStatusResponse } from './sitestatus.interface'
import { SiteStatusService } from './sitestatus.service'

@WebSocketGateway({
  cors: {
    origin: '*',
  },
})
@UseFilters(AppExceptionsFilter)
export class SiteStatusGateway {
  constructor(private readonly siteStatusService: SiteStatusService) {}

  @WebSocketServer()
  server: Server

  @UseInterceptors(SocketNamespaceInterceptor)
  @SubscribeMessage('sitestatus')
  async siteStatus(): Promise<Observable<SiteStatusResponse>> {
    const subscribeObservable = await this.siteStatusService.subscribeToSiteStatus()

    return subscribeObservable
  }
}
