import { Controller, Get } from '@nestjs/common';
import { ApiDefaultResponse, ApiOkResponse, ApiSecurity, ApiTags } from '@nestjs/swagger';
import { DashboardService } from './dashboard.service';
import { DefaultApiError } from 'src/exceptions/defaultResponse.exception';
import { SiteDiagramResponse } from './responses/dashboard.response';

@ApiTags('dashboard')
@ApiSecurity('bearerAuth')
@ApiDefaultResponse({ type: DefaultApiError })
@Controller('dashboard')
export class DashboardController {
  constructor(private readonly dashboardService: DashboardService) {}
  @Get('/site-diagram')
  @ApiOkResponse({ type: SiteDiagramResponse })
  async getSiteDiagram(): Promise<SiteDiagramResponse> {
    return await this.dashboardService.getSiteDiagramConfig();
  }
}
