import {
    CallHandler,
    ExecutionContext,
    Injectable,
    NestInterceptor,
    RequestTimeoutException,
} from '@nestjs/common'
import { Observable, throwError, TimeoutError } from 'rxjs'
import { catchError, timeout } from 'rxjs/operators'

import { AppEnvService } from '../environment/appEnv.service'

@Injectable()
export class TimeoutInterceptor implements NestInterceptor {
    constructor(private readonly appEnv: AppEnvService) {}
    intercept(context: ExecutionContext, next: CallHandler): Observable<any> {
        return next.handle().pipe(
            timeout(this.appEnv.getHttpTimeout()),
            catchError((err) => {
                if (err instanceof TimeoutError) {
                    return throwError(() => new RequestTimeoutException())
                }
                return throwError(() => err)
            })
        )
    }
}
