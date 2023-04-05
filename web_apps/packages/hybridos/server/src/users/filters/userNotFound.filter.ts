import { ArgumentsHost, Catch, ExceptionFilter, HttpStatus } from '@nestjs/common'
import { Response } from 'express'

import { UserNotFoundException } from '../exceptions/exceptions'

@Catch(UserNotFoundException)
export class UserNotFoundFilter implements ExceptionFilter {
    async catch(exception: UserNotFoundException, host: ArgumentsHost) {
        const ctx = host.switchToHttp()
        const response = ctx.getResponse<Response>()
        const statusCode = HttpStatus.NOT_FOUND

        response.status(statusCode).json({
            statusCode: statusCode,
            message: exception.message,
        })
    }
}
