import { Inject, Injectable } from '@nestjs/common';
import { map, merge, Observable } from 'rxjs';
import { FimsService } from '../../fims/fims.service';
import { FimsMsg, FIMS_SERVICE } from '../../fims/interfaces/fims.interface';
import { Site, Variable } from './params/override.params';
import { ErcotOverrideDto } from './ercotOverride.interface';
import { Request } from 'express';
import {
  IValidAccessTokenService,
  VALID_ACCESS_TOKEN_SERVICE,
} from 'src/auth/interfaces/validAccessToken.service.interface';

@Injectable()
export class ErcotOverrideService {
  constructor(
    @Inject(FIMS_SERVICE)
    private readonly fimsService: FimsService,
    @Inject(VALID_ACCESS_TOKEN_SERVICE)
    private readonly validAccessTokenService: IValidAccessTokenService,
  ) {}
  async getSiteNames(): Promise<string | Record<string, unknown>> {
    const fimsData = await this.fimsService.get('/fleet/features/ercotAs/sites');
    return fimsData.body;
  }
  async getVariableNames(): Promise<string | Record<string, unknown>> {
    const fimsData = await this.fimsService.get('/fleet/features/ercotAs/overridable');
    return fimsData.body;
  }
  async getVariableValues(site: Site): Promise<string | Record<string, unknown>> {
    const fimsData = await this.fimsService.get(`/fleet/features/ercotAs/${site.id}/overridable`);
    return fimsData.body;
  }
  async setOverrideValue(
    request: Request,
    site: Site,
    variable: Variable,
    body: { value: number | boolean },
  ): Promise<{ data: string }> {
    const token = this.validAccessTokenService.extractAccessTokenFromRequest(request);
    const user = this.validAccessTokenService.extractUserDataFromToken(token);

    this.fimsService.send({
      method: 'set',
      uri: `/fleet/features/ercotAs/${site.id}/${variable.name}`,
      replyto: '/ercot-override/variable-values',
      body: body,
      username: user.sub,
    });

    return { data: `SET request sent to /fleet/features/ercotAs/${site.id}/${variable.name}` };
  }
  getUriSpecificObservable = (siteId: string): Observable<ErcotOverrideDto> => {
    const fimsSubscribe = this.fimsService.subscribe(`/fleet/features/ercotAs/${siteId}`);

    const newObservable: Observable<ErcotOverrideDto> = fimsSubscribe.pipe(
      map((event) => {
        return { data: event.body } as ErcotOverrideDto;
      }),
    );
    return newObservable;
  };
}
