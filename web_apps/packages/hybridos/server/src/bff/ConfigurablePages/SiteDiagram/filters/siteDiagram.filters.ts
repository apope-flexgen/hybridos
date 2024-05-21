import { Catch, ArgumentsHost } from '@nestjs/common';
import { CoreWsExceptionsFilter } from '../../../../filters/core.ws.exception.filter';
import { ConfigNotFoundException } from '../../../../exceptions/configNotFound.exception';

@Catch(ConfigNotFoundException)
export class SiteDiagramConfigNotFoundExceptionsFilter extends CoreWsExceptionsFilter {
  catch(exception: any, host: ArgumentsHost) {
    this.logException(exception, host);

    this.buildResponse({ message: 'Site diagram configuration file not found' }, host);
  }
}
