import { ArgumentsHost, HttpStatus } from '@nestjs/common';

export type HttpResponseParams = {
  statusCode: HttpStatus;
  message: string;
};

export type WsResponseParams = {
  message: string;
};

export interface CoreExceptionFilter {
  logException(exception: any, host: ArgumentsHost): void;
  buildResponse(params: HttpResponseParams | WsResponseParams, host: ArgumentsHost): void;
}

export const DEFAULT_ERROR_MESSAGE = 'Server Error';
