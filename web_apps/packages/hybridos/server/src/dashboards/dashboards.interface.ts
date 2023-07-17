import { DashboardsResponse } from './responses'
import { AddDashboardRequest } from '../../../shared/types/dtos/dashboards.dto'

export interface IDashboardsService {
    getDashboards(): Promise<DashboardsResponse>
    postDashboards(data: AddDashboardRequest): Promise<DashboardsResponse>
}
