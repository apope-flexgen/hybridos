import { Catch, ArgumentsHost } from '@nestjs/common';
import { CoreWsExceptionsFilter } from '../../../../filters/core.ws.exception.filter';
import { DashboardConfigNotFoundException } from '../exceptions/dashboard.exceptions';

@Catch(DashboardConfigNotFoundException)
export class DashboardConfigNotFoundExceptionsFilter extends CoreWsExceptionsFilter {
  catch(exception: any, host: ArgumentsHost) {
    this.logException(exception, host);

    this.buildResponse({ message: 'Dashboard Config file not found' }, host);
  }
}
