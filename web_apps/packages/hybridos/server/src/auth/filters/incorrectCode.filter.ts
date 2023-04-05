import { ArgumentsHost, Catch, ExceptionFilter, HttpStatus } from '@nestjs/common'
import { Response } from 'express'
import { IncorrectTotpCodeException } from '../exceptions/incorrectTotpCode.exception'

@Catch(IncorrectTotpCodeException)
export class IncorrectCodeFilter implements ExceptionFilter {
    async catch(exception: IncorrectTotpCodeException, host: ArgumentsHost) {
        const ctx = host.switchToHttp()
        const response = ctx.getResponse<Response>()
        const status = HttpStatus.UNAUTHORIZED

        response.status(status).json({
            statusCode: status,
            message: 'Incorrect TOTP Code',
        })
    }
}
