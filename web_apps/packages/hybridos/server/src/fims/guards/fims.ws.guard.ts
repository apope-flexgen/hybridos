import { CanActivate, ExecutionContext, Injectable } from '@nestjs/common'
import { PermissionsService } from '../../permissions/permissions.service'

@Injectable()
export class FimsWebSocketGuard implements CanActivate {
    constructor(private readonly permissionsService: PermissionsService) {}
    canActivate(context: ExecutionContext) {
        const client = context.switchToWs().getClient()
        const data = context.switchToWs().getData()

        let uri: string
        let permissionLevel: string
        if (typeof data === 'string') {
            // subscribe
            uri = data
            permissionLevel = 'read'
        } else {
            uri = data.uri
            permissionLevel = data.method === 'get' ? 'read' : 'readWrite'
        }

        // do permissions check here
        return (
            client._socket.isValid &&
            this.permissionsService.ConfirmRoleAccess(client._socket.userRole, permissionLevel, uri)
        )
    }
}
