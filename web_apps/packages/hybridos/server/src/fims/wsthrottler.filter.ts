import { Catch, ArgumentsHost } from '@nestjs/common';
import { BaseWsExceptionFilter, WsException } from '@nestjs/websockets';

@Catch(WsException)
export class WsThrottleExceptionFilter extends BaseWsExceptionFilter {
  catch(exception: WsException, host: ArgumentsHost) {
    console.log('ws exception: ', exception.message);
    const client = host.switchToWs().getClient() as WebSocket;
    client.send(JSON.stringify({ error: exception.message }));
  }
}
