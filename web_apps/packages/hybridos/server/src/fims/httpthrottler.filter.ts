import { Catch, ArgumentsHost } from '@nestjs/common';
import { ThrottlerException } from '@nestjs/throttler';
import { Request, Response } from 'express';
import { CoreHttpExceptionsFilter } from '../filters/core.http.exception.filter';

@Catch(ThrottlerException)
export class HttpThrottleExceptionFilter extends CoreHttpExceptionsFilter {
  catch(exception: ThrottlerException, host: ArgumentsHost) {
    const ctx = host.switchToHttp();
    const response = ctx.getResponse<Response>();
    const request = ctx.getRequest<Request>();
    const status = exception.getStatus();

    response.status(status).json({
      statusCode: status,
      timestamp: new Date().toISOString(),
      path: request.url,
      error: 'Too many messages',
    });

    this.logException(exception, host);
  }
}
