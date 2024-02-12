import { SubscribeMessage, WebSocketGateway, WebSocketServer } from '@nestjs/websockets';
import { Observable, filter } from 'rxjs';
import { Server } from 'ws';
import { ConfigurablePageDTO } from 'shared/types/dtos/configurablePages.dto';
import { AssetsPageService } from './assetsPage.service';
import { EnableAssetPageControls } from 'src/decorators/assetPageControls.decorator';
import { SocketMessageBody } from 'src/decorators/socketMessageBody.decorator';
import { LockModeInterceptor } from './lockMode/lockMode.interceptor';
import { UseWsFilters } from '../../../decorators/ws.filters.decorator';
import { UseWsInterceptors } from '../../../decorators/ws.interceptors.decorator';
import { AssetsPagePruningInterceptor } from 'src/bff/ConfigurablePages/AssetsPage/interceptors/assetsPage.pruning.interceptor';

@WebSocketGateway({
  cors: {
    origin: '*',
  },
})
@UseWsFilters()
export class AssetsPageGateway {
  constructor(private readonly assetsPageService: AssetsPageService) {}

  @WebSocketServer()
  server: Server;
  @SubscribeMessage('assetsPage')
  @UseWsInterceptors(AssetsPagePruningInterceptor, LockModeInterceptor)
  async assetsPage(
    @SocketMessageBody() assetKey: string,
    @EnableAssetPageControls() enabledAssetPageControls: boolean,
  ): Promise<Observable<ConfigurablePageDTO>> {
    const subscribeObservable = await this.assetsPageService.subscribeToCategory(
      assetKey,
      enabledAssetPageControls,
    );

    return subscribeObservable;
  }
}
