import { Inject, Injectable } from '@nestjs/common';
import { User } from 'src/users/dtos/user.dto';
import { InvalidURIPermissions } from '../../exceptions/invalidPermissions.exception';
import { FIMS_SERVICE, IFimsService } from '../../fims/interfaces/fims.interface';
import {
  PermissionLevel,
  PERMISSIONS_SERVICE,
} from '../../permissions/interfaces/permission.interface';
import { PermissionsService } from '../../permissions/permissions.service';
import { AggregatedEndpointsService } from './aggregatedEndpoints.service';
import { LegacyRestPutResponse } from './responses/restput.response';

const GET_PERMISSION_LEVEL = PermissionLevel.READ;
const FIMS_GET_METHOD = 'get';
const FIMS_SET_METHOD = 'set';

@Injectable()
export class LegacyRestService {
  constructor(
    private readonly aggregatedEndpointsService: AggregatedEndpointsService,
    @Inject(PERMISSIONS_SERVICE) private readonly permissionsService: PermissionsService,
    @Inject(FIMS_SERVICE) private readonly fimsService: IFimsService,
  ) {}

  async get(uri: string, user: User) {
    if (!this.aggregatedEndpointsService.aggregatedEndpoints[uri]) {
      const sufficientPermissions = this.permissionsService.ConfirmRoleAccess(
        user,
        GET_PERMISSION_LEVEL,
        uri,
      );
      if (sufficientPermissions) {
        const response = await this.fimsService.send({
          method: FIMS_GET_METHOD,
          uri: uri,
          replyto: `/rest${uri}`,
          body: null,
          username: user.username,
        });

        if (typeof response.body === 'object') {
          return response.body;
        } else {
          // Naked value
          return {
            uri: uri,
            value: response.body,
          };
        }
      } else {
        throw new InvalidURIPermissions();
      }
    } else {
      return await this.aggregatedEndpointsService.getAggregatedEndpoint(uri, user);
    }
  }

  // Legacy implementation for BRP
  async set(uri: string, value: string, user: User): Promise<LegacyRestPutResponse> {
    let theValueCoerced: boolean | string;
    if (value === 'true' || value === 'false') {
      theValueCoerced = value !== 'false';
    } else {
      theValueCoerced = value;
    }
    const msg = {
      method: FIMS_SET_METHOD,
      uri: uri,
      replyto: null,
      body: theValueCoerced,
      username: user.username,
    };
    this.fimsService.send(msg);
    return {
      status: 202,
      statusString: 'accepted',
      method: 'PUT',
      uri: uri,
      value: theValueCoerced,
    };
  }
}
