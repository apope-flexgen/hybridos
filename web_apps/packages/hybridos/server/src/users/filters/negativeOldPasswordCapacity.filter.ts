import { ArgumentsHost, Catch, ExceptionFilter, HttpStatus } from '@nestjs/common'
import { Response } from 'express'

import { NegativeOldPasswordCapacityException } from '../exceptions/exceptions'

@Catch(NegativeOldPasswordCapacityException)
export class NegativeOldPasswordCapacityFilter implements ExceptionFilter {
    async catch(exception: NegativeOldPasswordCapacityException, host: ArgumentsHost) {
        const ctx = host.switchToHttp()
        const response = ctx.getResponse<Response>()
        const statusCode = HttpStatus.BAD_REQUEST

        response.status(statusCode).json({
            statusCode: statusCode,
            message: exception.message,
        })
    }
}
