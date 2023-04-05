import { ArgumentsHost, Catch, ExceptionFilter, HttpStatus } from '@nestjs/common'
import { Response } from 'express'
import { UnchangedPasswordException } from '../exceptions/unchangedPassword.exception'

@Catch(UnchangedPasswordException)
export class UnchangedPasswordFilter implements ExceptionFilter {
    async catch(exception: UnchangedPasswordException, host: ArgumentsHost) {
        const ctx = host.switchToHttp()
        const response = ctx.getResponse<Response>()
        const status = HttpStatus.BAD_REQUEST

        response.status(status).json({
            statusCode: status,
            message: 'New password must not match current password',
        })
    }
}
