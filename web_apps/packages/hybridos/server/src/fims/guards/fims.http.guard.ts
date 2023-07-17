import { CanActivate, ExecutionContext, Inject, Injectable } from '@nestjs/common';
import {
  PermissionLevel,
  PERMISSIONS_SERVICE,
} from '../../permissions/interfaces/permission.interface';
import { PermissionsService } from '../../permissions/permissions.service';

@Injectable()
export class FimsHttpPermissionsGuard implements CanActivate {
  constructor(
    @Inject(PERMISSIONS_SERVICE) private readonly permissionsService: PermissionsService,
  ) {}
  async canActivate(context: ExecutionContext): Promise<boolean> {
    const request = context.switchToHttp().getRequest();
    const user = request.user;

    let permissionLevel: PermissionLevel;
    let uri: string;

    if (request.method === 'GET') {
      // get from url request
      uri = request.query.uri;
      permissionLevel = PermissionLevel.READ;
    } else {
      // get from body
      uri = request.body.uri;
      permissionLevel = PermissionLevel.READ_WRITE;
    }

    // do permissions check here
    return this.permissionsService.ConfirmRoleAccess(user, permissionLevel, uri);
  }
}
