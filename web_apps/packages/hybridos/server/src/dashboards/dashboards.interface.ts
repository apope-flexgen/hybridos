import { DashboardsResponse } from "./responses"
import { AddDashboardRequest } from '../../../shared/types/dtos/dashboards.dto'
import { User } from "src/users/dtos/user.dto"

export interface IDashboardsService {
  getDashboards(): Promise<DashboardsResponse>
  postDashboards(data: AddDashboardRequest, user: User): Promise<DashboardsResponse>
}
