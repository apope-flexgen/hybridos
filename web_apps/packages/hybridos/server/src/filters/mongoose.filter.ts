import { ArgumentsHost, Catch, HttpStatus } from '@nestjs/common';
import { Response } from 'express';
import { MongoServerError } from 'mongodb';

// This weird import is necessary in order to catch errors thrown by mongoose.
import * as mongoose from 'mongoose';
import { CoreHttpExceptionsFilter } from './core.http.exception.filter';

@Catch(mongoose.mongo.MongoServerError)
export class MongoErrorFilter extends CoreHttpExceptionsFilter {
  async catch(exception: MongoServerError, host: ArgumentsHost) {
    let statusCode: number;
    let message: string;
    switch (exception.code) {
      case 11000: // duplicate key code
        statusCode = HttpStatus.BAD_REQUEST;
        message = `duplicate key(s): ${Object.keys(exception.keyValue)}`;
        break;
      default:
        statusCode = HttpStatus.INTERNAL_SERVER_ERROR;
        message = exception.message;
        break;
    }

    this.buildResponse({ statusCode, message }, host);
    this.logException(exception, host);
  }
}
