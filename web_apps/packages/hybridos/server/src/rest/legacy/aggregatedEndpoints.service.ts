import { Inject, Injectable, OnApplicationBootstrap } from '@nestjs/common';
import { AppEnvService } from '../../environment/appEnv.service';
import { FimsMsg, FIMS_SERVICE, IFimsService } from '../../fims/interfaces/fims.interface';
import { PermissionsService } from '../../permissions/permissions.service';
import { User } from 'src/users/dtos/user.dto';
import { Observable } from 'rxjs';
import {
  PermissionLevel,
  PERMISSIONS_SERVICE,
} from '../../permissions/interfaces/permission.interface';

const PERMISSION_LEVEL = PermissionLevel.READ;

@Injectable()
export class AggregatedEndpointsService implements OnApplicationBootstrap {
  public aggregatedEndpoints: {
    [parent_endpoint: string]: {
      [child_endpoint: string]: {
        observable: Observable<FimsMsg>;
        value: any;
      };
    };
  } = {};

  constructor(
    private readonly appEnvService: AppEnvService,
    @Inject(FIMS_SERVICE) private readonly fimsService: IFimsService,
    @Inject(PERMISSIONS_SERVICE) private readonly permissionsService: PermissionsService,
  ) {}

  async onApplicationBootstrap() {
    const aggEndpointsConfig = this.appEnvService.readAggregatedEndpoints();
    this.initAggregatedEndpoints(aggEndpointsConfig);
  }

  private async initAggregatedEndpoints(aggEndpointsConfig: any) {
    Object.keys(aggEndpointsConfig).forEach((parentURI: string) => {
      this.aggregatedEndpoints[parentURI] = {};

      aggEndpointsConfig[parentURI].forEach(async (childURI: string) => {
        this.aggregatedEndpoints[parentURI][childURI] = {
          observable: undefined,
          value: {},
        };
        // Do a fims get on the childURI to populate initial data.
        this.getChildURI(parentURI, childURI);
        // Get observable for childURI and subscribe for fims pub updates.
        this.getObservableAndSubscribeChildURI(parentURI, childURI);
      });
    });
  }

  private async getChildURI(parentURI: string, childURI: string): Promise<void> {
    const getMsg = await this.fimsService.get(`${parentURI}${childURI}`);
    this.aggregatedEndpoints[parentURI][childURI].value = getMsg.body;
  }

  private getObservableAndSubscribeChildURI(parentURI: string, childURI: string): void {
    this.aggregatedEndpoints[parentURI][childURI].observable = this.fimsService.subscribe(
      `${parentURI}${childURI}`,
    );
    this.aggregatedEndpoints[parentURI][childURI].observable.subscribe((msg: FimsMsg) => {
      this.aggregatedEndpoints[parentURI][childURI].value = Object.assign(
        this.aggregatedEndpoints[parentURI][childURI].value,
        msg.body,
      );
    });
  }

  public async getAggregatedEndpoint(parentURI: string, user: User) {
    const result = Object.keys(this.aggregatedEndpoints[parentURI])
      .filter((childURI: string) => {
        const fullURI = `${parentURI}${childURI}`;

        const sufficientPermissions = this.permissionsService.ConfirmRoleAccess(
          user,
          PERMISSION_LEVEL,
          fullURI,
        );
        return sufficientPermissions;
      })
      .map(async (childURI: string) => {
        return {
          [childURI]: this.aggregatedEndpoints[parentURI][childURI].value,
        };
      });

    if (result.length === 0) return {};

    const formatted = result.reduce((prev, cur) => {
      return { ...prev, ...cur };
    });

    return formatted;
  }
}
