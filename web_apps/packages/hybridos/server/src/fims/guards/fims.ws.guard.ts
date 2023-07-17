import { CanActivate, ExecutionContext, Inject, Injectable } from '@nestjs/common';
import {
  PermissionLevel,
  PERMISSIONS_SERVICE,
} from '../../permissions/interfaces/permission.interface';
import { PermissionsService } from '../../permissions/permissions.service';

@Injectable()
export class FimsWebSocketGuard implements CanActivate {
  constructor(
    @Inject(PERMISSIONS_SERVICE) private readonly permissionsService: PermissionsService,
  ) {}
  canActivate(context: ExecutionContext) {
    const client = context.switchToWs().getClient();
    const data = context.switchToWs().getData().data;

    let uri: string;
    let permissionLevel: PermissionLevel;
    if (typeof data === 'string') {
      // subscribe
      uri = data;
      permissionLevel = PermissionLevel.READ;
    } else {
      uri = data.uri;
      permissionLevel = data.method === 'get' ? PermissionLevel.READ : PermissionLevel.READ_WRITE;
    }

    // do permissions check here
    return (
      client._socket.isValid &&
      this.permissionsService.ConfirmRoleAccess(
        {
          role: client._socket.userRole,
          username: client._socket.username,
        },
        permissionLevel,
        uri,
      )
    );
  }
}
