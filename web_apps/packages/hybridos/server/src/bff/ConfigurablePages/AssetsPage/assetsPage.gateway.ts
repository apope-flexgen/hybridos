import {
    MessageBody,
    SubscribeMessage,
    WebSocketGateway,
    WebSocketServer,
} from '@nestjs/websockets'
import { Observable } from 'rxjs'
import { Server } from 'ws'
import { ConfigurablePageDTO } from 'shared/types/dtos/configurablePages.dto'
import { AssetsPageService } from './assetsPage.service'
import { EnableAssetPageControls } from 'src/decorators/assetPageControls.decorator'

@WebSocketGateway({
    cors: {
        origin: '*',
    },
})
export class AssetsPageGateway {
    constructor(private readonly assetsPageService: AssetsPageService) {}
    @WebSocketServer()
    server: Server
    @SubscribeMessage('assetsPage')
    async assetsPage(
        @MessageBody() category: string,
        @EnableAssetPageControls() enabledAssetPageControls: boolean,
    ): Promise<Observable<ConfigurablePageDTO>> {
        // TODO: find a better way to determine this
        const isClothed = category !== 'ess'
        const subscribeObservable = await this.assetsPageService.subscribeToCategory(
            category,
            isClothed,
            enabledAssetPageControls,
        )

        return subscribeObservable
    }
}
