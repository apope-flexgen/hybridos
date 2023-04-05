import { ArgumentsHost, Catch, ExceptionFilter, HttpStatus } from '@nestjs/common'
import { Response } from 'express'

// TODO: michael should review this. this import did not work after merging, so I changed it
// not sure if it is still working as intended
// This weird import is necessary in order to catch errors thrown by mongoose.
// import { MongoServerError } from 'mongoose/node_modules/mongodb';
import { MongoServerError } from 'mongodb'

@Catch(MongoServerError)
export class MongoErrorFilter implements ExceptionFilter {
    async catch(exception: MongoServerError, host: ArgumentsHost) {
        let statusCode: number
        let message: string
        switch (exception.code) {
            case 11000: // duplicate key code
                statusCode = HttpStatus.BAD_REQUEST
                message = `duplicate key(s): ${Object.keys(exception.keyValue)}`
                break
            default:
                statusCode = HttpStatus.INTERNAL_SERVER_ERROR
                message = exception.message
                break
        }
        const ctx = host.switchToHttp()
        const response = ctx.getResponse<Response>()

        response.status(statusCode).json({
            statusCode: statusCode,
            message: message,
        })
    }
}
