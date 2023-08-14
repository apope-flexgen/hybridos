import { ArgumentsHost, Catch, HttpStatus } from '@nestjs/common';
import { CoreHttpExceptionsFilter } from '../../filters/core.http.exception.filter';
import { UserNotFoundException } from '../../users/exceptions/exceptions';

@Catch(UserNotFoundException)
export class UserNotFoundFilter extends CoreHttpExceptionsFilter {
  async catch(exception: UserNotFoundException, host: ArgumentsHost) {
    const statusCode = HttpStatus.UNAUTHORIZED;
    const message = 'Unauthorized';

    this.buildResponse({ statusCode, message }, host);
    this.logException(exception, host);
  }
}
