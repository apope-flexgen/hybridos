import { Catch, ArgumentsHost, HttpException } from '@nestjs/common';

import { CoreHttpExceptionsFilter } from './core.http.exception.filter';

@Catch(HttpException)
export class HttpExceptionsFilter extends CoreHttpExceptionsFilter {
  catch(exception: HttpException, host: ArgumentsHost) {
    const statusCode = exception.getStatus();
    const message = exception.message;

    this.buildResponse({ statusCode, message }, host);
    this.logException(exception, host);
  }
}
