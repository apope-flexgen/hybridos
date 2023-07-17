import {
    SubscribeMessage,
    WebSocketGateway,
    WebSocketServer,
} from '@nestjs/websockets'
import { Observable, tap } from 'rxjs'
import { Server } from 'ws'
import { ConfigurablePageDTO } from 'shared/types/dtos/configurablePages.dto'
import { AssetsPageService } from './assetsPage.service'
import { EnableAssetPageControls } from 'src/decorators/assetPageControls.decorator'
import { SocketNamespaceInterceptor } from 'src/interceptors/socketNamespace.interceptor'
import { UseInterceptors } from '@nestjs/common'
import { SocketMessageBody } from 'src/decorators/socketMessageBody.decorator'
import { UseFilters } from '@nestjs/common'
import { AppExceptionsFilter } from 'src/filters/all-exceptions.filter'
import { LockModeInterceptor } from './lockMode/lockMode.interceptor'

@WebSocketGateway({
    cors: {
        origin: '*',
    },
})
@UseFilters(AppExceptionsFilter)
export class AssetsPageGateway {
    constructor(private readonly assetsPageService: AssetsPageService) {}

    @WebSocketServer()
    server: Server
    @SubscribeMessage('assetsPage')
    @UseInterceptors(SocketNamespaceInterceptor, LockModeInterceptor)
    async assetsPage(
        @SocketMessageBody() assetKey: string,
        @EnableAssetPageControls() enabledAssetPageControls: boolean
    ): Promise<Observable<ConfigurablePageDTO>> {
        const subscribeObservable =
            await this.assetsPageService.subscribeToCategory(
                assetKey,
                enabledAssetPageControls
            )

        return subscribeObservable
    }
}
