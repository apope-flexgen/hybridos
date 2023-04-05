import { MessageBody,SubscribeMessage, WebSocketGateway, WebSocketServer } from '@nestjs/websockets'
import { Observable, tap } from 'rxjs'
import { Server } from 'ws'
import { VariableOverrideDto } from './ercotOverride.interface'
import { VariableOverrideService } from './ercotOverride.service'

@WebSocketGateway({
    cors: {
        origin: '*',
    },
})
export class VariableOverrideGateway {
    constructor(private readonly variableOverrideService: VariableOverrideService) {}
    @WebSocketServer()
    server: Server
    @SubscribeMessage('variableOverride')
    async variableOverride(
        @MessageBody() siteId: string
    ): Promise<Observable<VariableOverrideDto>> {
        const variableOverrideDataStream = await this.variableOverrideService.getUriSpecificObservable(siteId)

        return variableOverrideDataStream
    }
}
