import { ArgumentsHost, Catch, HttpStatus } from '@nestjs/common';
import { CoreHttpExceptionsFilter } from '../../filters/core.http.exception.filter';

import { OldPasswordMatchException } from '../exceptions/exceptions';

@Catch(OldPasswordMatchException)
export class OldPasswordMatchFilter extends CoreHttpExceptionsFilter {
  async catch(exception: OldPasswordMatchException, host: ArgumentsHost) {
    const statusCode = HttpStatus.BAD_REQUEST;
    const message = exception.message;

    this.buildResponse({ statusCode, message }, host);
    this.logException(exception, host);
  }
}
