import { Injectable, Logger, NestMiddleware } from '@nestjs/common'
import { Request, Response, NextFunction } from 'express'
import { LoggingService } from './logging.service'
import { LoggerFilter } from './logging_filter/logger-filter.service'
import { LogText } from './log_text/log-text'
const crypto = require('crypto')

@Injectable()
export class LoggerMiddleware implements NestMiddleware {
    private myLogger = new LoggingService()
    private loggerFilter = new LoggerFilter()
    use(request: Request, response: Response, next: NextFunction): void {
        const { ip, method, originalUrl, hostname } = request
        const userAgent = request.get('user-agent') || ''
        response.on('finish', () => {
            const { statusCode } = response
            const contentLength = response.get('content-length')

            if (this.loggerFilter.requestMayBeLogged(method, statusCode, originalUrl)) {
                const statusCodeString: string = statusCode.toString(10)
                let log = <LogText>{
                    verb: method,
                    url: originalUrl,
                    code: statusCodeString,
                    length: contentLength,
                    useragent: userAgent,
                    ip: ip,
                    hostname: hostname,
                }
                this.myLogger.log(log, '[HTTP]')
            }
        })
        next()
    }
}
