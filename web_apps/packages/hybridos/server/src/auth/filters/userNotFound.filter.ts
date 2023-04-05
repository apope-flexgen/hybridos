import { ArgumentsHost, Catch, ExceptionFilter, HttpStatus } from '@nestjs/common'
import { Response } from 'express'
import { UserNotFoundException } from '../../users/exceptions/exceptions'

@Catch(UserNotFoundException)
export class UserNotFoundFilter implements ExceptionFilter {
    async catch(exception: UserNotFoundException, host: ArgumentsHost) {
        const ctx = host.switchToHttp()
        const response = ctx.getResponse<Response>()
        const status = HttpStatus.UNAUTHORIZED

        response.status(status).json({
            statusCode: status,
            message: 'Unauthorized',
        })
    }
}
