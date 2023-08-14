import { ArgumentsHost, Catch, HttpStatus } from '@nestjs/common';
import { CoreHttpExceptionsFilter } from '../../filters/core.http.exception.filter';

import { UserNotFoundException } from '../exceptions/exceptions';

@Catch(UserNotFoundException)
export class UserNotFoundFilter extends CoreHttpExceptionsFilter {
  async catch(exception: UserNotFoundException, host: ArgumentsHost) {
    const statusCode = HttpStatus.NOT_FOUND;
    const message = exception.message;

    this.buildResponse({ statusCode, message }, host);
    this.logException(exception, host);
  }
}
