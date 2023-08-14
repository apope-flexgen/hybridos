import { Catch, ArgumentsHost, ExceptionFilter, HttpStatus, HttpException } from '@nestjs/common';
import { Response } from 'express';
import { LoggingService } from 'src/logging/logging.service';
import { LogText } from 'src/logging/log_text/log-text';
import {
  CoreExceptionFilter,
  DEFAULT_ERROR_MESSAGE,
  HttpResponseParams,
} from './core.exception.filter.interface';

@Catch()
export class CoreHttpExceptionsFilter implements CoreExceptionFilter, ExceptionFilter {
  constructor(private readonly loggingService: LoggingService) {}

  catch(exception: any, host: ArgumentsHost) {
    const statusCode = HttpStatus.INTERNAL_SERVER_ERROR;
    const message = DEFAULT_ERROR_MESSAGE;

    this.buildResponse({ statusCode, message }, host);
    this.logException(exception, host);
  }

  logException = (exception: any, host: ArgumentsHost) => {
    const context = host.getType();
    const target = host.switchToHttp().getRequest().url;

    const log: LogText = {
      message: exception.message,
      stack: exception.stack,
      url: target,
    };
    this.loggingService.error(log, context);
  };

  buildResponse = ({ statusCode, message }: HttpResponseParams, host: ArgumentsHost) => {
    const ctx = host.switchToHttp();
    const response = ctx.getResponse<Response>();

    response.status(statusCode).json({
      statusCode,
      message,
    });
  };
}
