import { ArgumentsHost, Catch, HttpStatus } from '@nestjs/common';
import { CoreHttpExceptionsFilter } from '../../filters/core.http.exception.filter';
import { UnchangedPasswordException } from '../exceptions/unchangedPassword.exception';

@Catch(UnchangedPasswordException)
export class UnchangedPasswordFilter extends CoreHttpExceptionsFilter {
  async catch(exception: UnchangedPasswordException, host: ArgumentsHost) {
    const statusCode = HttpStatus.BAD_REQUEST;
    const message = 'New password must not match current password';

    this.buildResponse({ statusCode, message }, host);
    this.logException(exception, host);
  }
}
