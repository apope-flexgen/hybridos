import { ExecutionContext, Injectable, UnauthorizedException } from '@nestjs/common'
import { Reflector } from '@nestjs/core'
import { AuthGuard } from '@nestjs/passport'
import { IS_BASIC_AUTH_KEY } from 'src/decorators/basicAuth.decorator';

import { IS_PUBLIC_KEY } from '../../decorators/public.decorator';

@Injectable()
export class AccessTokenAuthGuard extends AuthGuard('jwt') {
    constructor(private reflector: Reflector) {
        super()
    }

    canActivate(context: ExecutionContext) {
        const isPublic = this.reflector.getAllAndOverride<boolean>(IS_PUBLIC_KEY, [
            context.getHandler(),
            context.getClass(),
        ])
        const isBasicAuth = this.reflector.getAllAndOverride<boolean>(IS_BASIC_AUTH_KEY, [
            context.getHandler(),
            context.getClass(),
        ])
        if (isBasicAuth || isPublic) {
            return true
        }
        return super.canActivate(context);
    }

    handleRequest(err: any, user: any, info: any) {
        if (err || !user) {
            const message = info ? info.message : 'Unauthorized'
            throw err || new UnauthorizedException({ message: message, statusCode: 401 })
        }
        return user
    }
}
