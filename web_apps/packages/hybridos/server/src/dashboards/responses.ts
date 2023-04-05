import { ApiProperty } from '@nestjs/swagger'
import { DashboardsDescriptions } from './dashboards.constants'

export class DashboardsResponse {
  @ApiProperty({ description: DashboardsDescriptions.dashboardsResponse })
  dashboards: string | Record<string, unknown>
}
