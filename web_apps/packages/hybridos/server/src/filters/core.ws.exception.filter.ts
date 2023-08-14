import { Catch, ArgumentsHost } from '@nestjs/common';
import { BaseWsExceptionFilter } from '@nestjs/websockets';
import { LoggingService } from 'src/logging/logging.service';
import { LogText } from 'src/logging/log_text/log-text';
import {
  CoreExceptionFilter,
  DEFAULT_ERROR_MESSAGE,
  WsResponseParams,
} from './core.exception.filter.interface';

@Catch()
export class CoreWsExceptionsFilter extends BaseWsExceptionFilter implements CoreExceptionFilter {
  constructor(private readonly loggingService: LoggingService) {
    super();
  }

  catch(exception: any, host: ArgumentsHost) {
    this.logException(exception, host);

    this.buildResponse({ message: DEFAULT_ERROR_MESSAGE }, host);
  }

  logException = (exception: any, host: ArgumentsHost) => {
    const context = host.getType();

    // 'ws' context type has no url. getData() is a
    // meaningful way to determine the target resource.
    const target = host.switchToWs().getData();

    const log: LogText = {
      message: exception.message,
      stack: exception.stack,
      url: target,
    };
    this.loggingService.error(log, context);
  };

  buildResponse = ({ message }: WsResponseParams, host: ArgumentsHost) => {
    const client = host.switchToWs().getClient();

    const event = {
      namespace: 'exception',
      data: { message },
    };
    client.send(JSON.stringify(event));
  };
}
