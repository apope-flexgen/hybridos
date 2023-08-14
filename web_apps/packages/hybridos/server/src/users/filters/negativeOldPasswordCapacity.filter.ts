import { ArgumentsHost, Catch, HttpStatus } from '@nestjs/common';
import { CoreHttpExceptionsFilter } from '../../filters/core.http.exception.filter';

import { NegativeOldPasswordCapacityException } from '../exceptions/exceptions';

@Catch(NegativeOldPasswordCapacityException)
export class NegativeOldPasswordCapacityFilter extends CoreHttpExceptionsFilter {
  async catch(exception: NegativeOldPasswordCapacityException, host: ArgumentsHost) {
    const statusCode = HttpStatus.BAD_REQUEST;
    const message = exception.message;

    this.buildResponse({ statusCode, message }, host);
    this.logException(exception, host);
  }
}
