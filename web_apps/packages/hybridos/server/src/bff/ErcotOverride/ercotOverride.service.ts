import { Inject, Injectable } from '@nestjs/common'
import { map, merge, Observable } from 'rxjs';
import { FimsService } from '../../fims/fims.service'
import { FimsMsg, FIMS_SERVICE } from '../../fims/interfaces/fims.interface'
import { EditOverrideParams } from './params/editOverride.params';
import { VariableParams } from './params/variableNames.params';
import { VariableOverrideDto } from './ercotOverride.interface';
import { IValidJWTService, VALID_JWT_SERVICE } from 'src/auth/interfaces/validJWT.service.interface'
import { Request } from 'express';

@Injectable()
export class VariableOverrideService {
    constructor(
        @Inject(FIMS_SERVICE) 
        private readonly fimsService: FimsService,
        @Inject(VALID_JWT_SERVICE)
        private readonly validJwtService: IValidJWTService,
    ) { }
    async getSiteNames(): Promise<string | Record<string, unknown>> {
        const fimsData = await this.fimsService.get('/fleet/features/ercotAs/sites');
        return fimsData.body
    }
    async getVariableNames(): Promise<string | Record<string, unknown>> {
        const fimsData = await this.fimsService.get('/fleet/features/ercotAs/overridable');
        return fimsData.body
    }
    async getVariableValues(params: VariableParams): Promise<string | Record<string, unknown>> {
        const fimsData = await this.fimsService.get(`/fleet/features/ercotAs/${params.siteId}/overridable`);
        return fimsData.body
    }
    async setOverrideValue(
        request: Request, 
        params: EditOverrideParams, 
        body: {data: number | boolean}
    ): Promise<{ data: string }> 
    {        
        const token = this.validJwtService.extractAccessTokenFromRequest(request)
        const user = this.validJwtService.extractUserDataFromToken(token);

        this.fimsService.send({
            method: 'set',
            uri: `/fleet/features/ercotAs/${params.siteId}/${params.variableName}`,
            replyto: '/ercot-override/variable-values',
            body: body,
            username: user.sub
        })

        return { data: `SET request sent to /fleet/features/ercotAs/${params.siteId}/${params.variableName}` }
    }
    getUriSpecificObservable = (
        siteId: string,
    ): Observable<VariableOverrideDto> => {
        const fimsSubscribe = this.fimsService.subscribe(`/fleet/features/ercotAs/${siteId}`)

        const newObservable: Observable<VariableOverrideDto> = fimsSubscribe.pipe(
            map((event) => {
                return { data: event.body } as VariableOverrideDto
            })
        )
        return newObservable
    }
}
