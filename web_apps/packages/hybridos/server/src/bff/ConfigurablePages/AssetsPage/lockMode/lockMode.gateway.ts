import { Inject } from '@nestjs/common';
import { SubscribeMessage, WebSocketGateway, WebSocketServer } from '@nestjs/websockets';
import { Server } from 'ws';
import { FIMS_SERVICE, FimsMsg, IFimsService } from 'src/fims/interfaces/fims.interface';
import { LockModeDTO } from './dto/lockMode.dto';
import { UserFromSocket } from '../../../../decorators/userFromSocket.decorator';
import { User } from 'shared/types/dtos/auth.dto';
import { LockModeService } from './lockMode.service';
import { SocketMessageBody } from '../../../../decorators/socketMessageBody.decorator';
import { EnableAssetPageControls } from '../../../../decorators/assetPageControls.decorator';
import { UseWsFilters } from '../../../../decorators/ws.filters.decorator';

@WebSocketGateway({
  cors: {
    origin: '*',
  },
})
@UseWsFilters()
export class LockModeGateway {
  constructor(
    @Inject(FIMS_SERVICE)
    private readonly fimsService: IFimsService,
    private readonly lockModeService: LockModeService,
  ) {}
  @WebSocketServer()
  server: Server;
  @SubscribeMessage('lockMode')
  async lockMode(
    @SocketMessageBody() data: LockModeDTO,
    @UserFromSocket() { username }: User,
    @EnableAssetPageControls() enabledAssetPageControls: boolean,
  ) {
    try {
      const canChangeLockMode = await this.lockModeService.canChangeLockMode(
        data.uri,
        username,
        enabledAssetPageControls,
      );

      if (canChangeLockMode) {
        const msg: FimsMsg = {
          method: 'set',
          uri: `${data.uri}/lock_mode`,
          replyto: '/response',
          body: data.value,
          username: 'web_ui',
        };

        this.fimsService.send(msg);
        await this.lockModeService.postLockModeUpdate(data.uri, data.value, username);
      }
    } catch (e) {
      console.log('request failed: ', e);
    }
  }
}
