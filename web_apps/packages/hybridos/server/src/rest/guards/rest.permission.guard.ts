import { CanActivate, ExecutionContext, Inject, Injectable } from '@nestjs/common';
import { PermissionsService } from '../../permissions/permissions.service';
import { Reflector } from '@nestjs/core';
import { IS_AGGREGATED_ENDPOINTS_KEY } from 'src/decorators/aggregatedEndpoints.decorator';
import {
  PermissionLevel,
  PERMISSIONS_SERVICE,
} from '../../permissions/interfaces/permission.interface';
import { REST_API_PREFIX } from '../interfaces/rest.interface';

@Injectable()
export class RestPermissionsGuard implements CanActivate {
  constructor(
    @Inject(PERMISSIONS_SERVICE) private readonly permissionsService: PermissionsService,
    private readonly reflector: Reflector,
  ) {}

  async canActivate(context: ExecutionContext): Promise<boolean> {
    const isAggregatedEndpoints = this.reflector.getAllAndOverride<boolean>(
      IS_AGGREGATED_ENDPOINTS_KEY,
      [context.getHandler(), context.getClass()],
    );
    if (isAggregatedEndpoints) {
      // Requested method may include aggregated endpoints, so check permissions later.
      return true;
    }
    const request = context.switchToHttp().getRequest();
    const user = request.user;

    let permissionLevel: PermissionLevel;
    let url: string = request.url;
    let uri: string = url.replace(`/${REST_API_PREFIX}`, '');

    permissionLevel = request.method === 'GET' ? PermissionLevel.READ : PermissionLevel.READ_WRITE;

    // do permissions check here
    return this.permissionsService.ConfirmRoleAccess(user, permissionLevel, uri);
  }
}
