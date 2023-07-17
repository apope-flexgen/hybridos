import { CanActivate, ExecutionContext, Inject, Injectable } from '@nestjs/common';
import { PermissionsService } from '../../../permissions/permissions.service';
import { Reflector } from '@nestjs/core';
import { IS_AGGREGATED_ENDPOINTS_KEY } from 'src/decorators/aggregatedEndpoints.decorator';
import {
  PermissionLevel,
  PERMISSIONS_SERVICE,
} from '../../../permissions/interfaces/permission.interface';
import {
  LEGACY_REST_API_PREFIX,
  LEGACY_REST_API_PREFIX_V1,
} from '../interfaces/rest.legacy.interface';

@Injectable()
export class LegacyRestPermissionsGuard implements CanActivate {
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
    let uri: string = url
      .replace(`/${LEGACY_REST_API_PREFIX_V1}`, '')
      .replace(`/${LEGACY_REST_API_PREFIX}`, '');

    if (request.method === 'PUT') {
      // remove the last element of the uri, which will be the value being sent to fims
      uri = uri.split('/').slice(0, -1).join('/');
    }

    permissionLevel = request.method === 'GET' ? PermissionLevel.READ : PermissionLevel.READ_WRITE;

    // do permissions check here
    return this.permissionsService.ConfirmRoleAccess(user, permissionLevel, uri);
  }
}
