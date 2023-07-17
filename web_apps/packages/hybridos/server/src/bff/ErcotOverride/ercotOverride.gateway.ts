import { UseInterceptors } from '@nestjs/common'
import {
    SubscribeMessage,
    WebSocketGateway,
    WebSocketServer,
} from '@nestjs/websockets'
import { SocketMessageBody } from 'src/decorators/socketMessageBody.decorator'
import { SocketNamespaceInterceptor } from 'src/interceptors/socketNamespace.interceptor'
import { UseFilters } from '@nestjs/common'
import { Observable } from 'rxjs'
import { AppExceptionsFilter } from 'src/filters/all-exceptions.filter'
import { Server } from 'ws'
import { ErcotOverrideDto } from './ercotOverride.interface'
import { ErcotOverrideService } from './ercotOverride.service'

@WebSocketGateway({
    cors: {
        origin: '*',
    },
})
@UseFilters(AppExceptionsFilter)
export class ErcotOverrideGateway {
    constructor(
        private readonly ercotOverrideService: ErcotOverrideService
    ) {}

    @WebSocketServer()
    server: Server

    @SubscribeMessage('ercotOverride')
    @UseInterceptors(SocketNamespaceInterceptor)
    ercotOverride(
        @SocketMessageBody() siteId: string
    ): Observable<ErcotOverrideDto> {
        const ercotOverrideDataStream =
            this.ercotOverrideService.getUriSpecificObservable(siteId)

        return ercotOverrideDataStream
    }
}
