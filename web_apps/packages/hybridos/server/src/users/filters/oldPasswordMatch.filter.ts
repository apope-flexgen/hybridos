import { ArgumentsHost, Catch, ExceptionFilter, HttpStatus } from '@nestjs/common'
import { Response } from 'express'

import { OldPasswordMatchException } from '../exceptions/exceptions'

@Catch(OldPasswordMatchException)
export class OldPasswordMatchFilter implements ExceptionFilter {
    async catch(exception: OldPasswordMatchException, host: ArgumentsHost) {
        const ctx = host.switchToHttp()
        const response = ctx.getResponse<Response>()
        const statusCode = HttpStatus.BAD_REQUEST

        response.status(statusCode).json({
            statusCode: statusCode,
            message: exception.message,
        })
    }
}
