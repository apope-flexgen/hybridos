import { Injectable, Inject } from '@nestjs/common'
import { AddDashboardRequest } from 'shared/types/dtos/dashboards.dto'
import { DBI_SERVICE, DBI_URIs } from 'src/dbi/dbi.interface'
import { IDBIService } from 'src/dbi/dbi.interface'
import { IDashboardsService } from './dashboards.interface'
import { DashboardsResponse } from './responses'

@Injectable()
export class DashboardsService implements IDashboardsService {
    constructor(
        @Inject(DBI_SERVICE)
        private readonly dbiService: IDBIService
    ) {}

    getDashboards = async (): Promise<DashboardsResponse> =>
        this.dbiService.getFromDBI(DBI_URIs.UI_Config_Dashboard)

    postDashboards = async (
        data: AddDashboardRequest
    ): Promise<DashboardsResponse> =>
        this.dbiService.postToDBI(DBI_URIs.UI_Config_Dashboard, data)
}
