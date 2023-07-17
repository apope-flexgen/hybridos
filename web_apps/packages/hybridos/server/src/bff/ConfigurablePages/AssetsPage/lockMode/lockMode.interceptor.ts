import { CallHandler, ExecutionContext, Injectable, NestInterceptor } from '@nestjs/common';
import { Observable, switchMap } from 'rxjs';
import { Roles } from 'shared/types/api/Users/Users.types';
import { ConfigurablePageDTO } from 'shared/types/dtos/configurablePages.dto';
import { VALID_ASSET_CONTROL_ROLES } from '../../../../decorators/assetPageControls.decorator';
import { LockModeService } from './lockMode.service';

@Injectable()
export class LockModeInterceptor implements NestInterceptor {
  constructor(private readonly lockModeService: LockModeService) {}

  intercept(
    context: ExecutionContext,
    next: CallHandler<any>,
  ): Observable<ConfigurablePageDTO> | Promise<Observable<ConfigurablePageDTO>> {
    const username = context.switchToWs().getClient()._socket.username;
    const role = context.switchToWs().getClient()._socket.userRole as Roles;
    const enabledAssetPageControls = VALID_ASSET_CONTROL_ROLES.get(role);

    return next.handle().pipe(
      switchMap(async (data: ConfigurablePageDTO) => {
        for (const URI of Object.keys(data.displayGroups)) {
          const lockModeObject = await this.lockModeService.getLockModeObject(
            URI,
            username,
            enabledAssetPageControls,
          );
          const userHasAccess = lockModeObject?.lockModeButtonEnabled;

          if (lockModeObject && data.displayGroups[URI].control['maint_mode']) {
            data.displayGroups[URI].control['maint_mode'].state.extraProps = {
              ...data.displayGroups[URI].control['maint_mode'].state?.extraProps,
              lockMode: lockModeObject,
            };
          }

          // only overwrite access if userHasAccess is false, not undefined
          if (userHasAccess === false) {
            for (const control of Object.values(data.displayGroups[URI].control)) {
              control.state.enabled = userHasAccess;
            }
          }
        }

        return data;
      }),
    );
  }
}
