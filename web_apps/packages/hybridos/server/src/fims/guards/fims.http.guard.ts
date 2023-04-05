import { CanActivate, ExecutionContext, Injectable } from '@nestjs/common'
import { PermissionsService } from '../../permissions/permissions.service'

@Injectable()
export class FimsHttpPermissionsGuard implements CanActivate {
    constructor(private readonly permissionsService: PermissionsService) {}
    async canActivate(context: ExecutionContext): Promise<boolean> {
        const request = context.switchToHttp().getRequest()
        const user = request.user

        let permissionLevel: string
        let uri: string

        if (request.method === 'GET') {
            // get from url request
            uri = request.query.uri
            permissionLevel = 'read'
        } else {
            // get from body
            uri = request.body.uri
            permissionLevel = 'readWrite'
        }

        // do permissions check here
        return this.permissionsService.ConfirmRoleAccess(user.role, permissionLevel, uri)
    }
}
