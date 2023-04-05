import { Catch, ArgumentsHost } from '@nestjs/common'
import { ThrottlerException } from '@nestjs/throttler'
import { BaseExceptionFilter } from '@nestjs/core'
import { Request, Response } from 'express'

@Catch(ThrottlerException)
export class HttpThrottleExceptionFilter extends BaseExceptionFilter {
    catch(exception: ThrottlerException, host: ArgumentsHost) {
        const ctx = host.switchToHttp()
        const response = ctx.getResponse<Response>()
        const request = ctx.getRequest<Request>()
        const status = exception.getStatus()

        response.status(status).json({
            statusCode: status,
            timestamp: new Date().toISOString(),
            path: request.url,
            error: 'Too many messages',
        })
    }
}
