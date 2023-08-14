import { ArgumentsHost, Catch, HttpStatus } from '@nestjs/common';
import { CoreHttpExceptionsFilter } from '../../filters/core.http.exception.filter';
import { IncorrectTotpCodeException } from '../exceptions/incorrectTotpCode.exception';

@Catch(IncorrectTotpCodeException)
export class IncorrectCodeFilter extends CoreHttpExceptionsFilter {
  async catch(exception: IncorrectTotpCodeException, host: ArgumentsHost) {
    const statusCode = HttpStatus.UNAUTHORIZED;
    const message = 'Incorrect TOTP Code';

    this.buildResponse({ statusCode, message }, host);
    this.logException(exception, host);
  }
}
