import { Injectable, Inject } from '@nestjs/common'
import { AddDashboardRequest } from 'shared/types/dtos/dashboards.dto';
import { DBI_SERVICE } from 'src/dbi/dbi.constants';
import { IDBIService } from 'src/dbi/dbi.interface';
import { User } from 'src/users/dtos/user.dto';
import { IDashboardsService } from './dashboards.interface';
import { DashboardsResponse } from './responses';

@Injectable()
export class DashboardsService implements IDashboardsService {
    constructor(
        @Inject(DBI_SERVICE)
        private readonly dbiService: IDBIService,
    ) { }

    getDashboards = async (): Promise<DashboardsResponse> => this.dbiService.getUIConfigDashboards();

    postDashboards = async (data: AddDashboardRequest, user: User): Promise<DashboardsResponse> => this.dbiService.postUIConfigDashboards(data, user);
}
