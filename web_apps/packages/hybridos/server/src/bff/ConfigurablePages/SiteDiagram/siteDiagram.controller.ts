import { Controller, Get, Inject } from '@nestjs/common';
import { ApiDefaultResponse, ApiSecurity, ApiTags } from '@nestjs/swagger';
import { DefaultApiError } from 'src/exceptions/defaultResponse.exception';
import { SITE_DIAGRAM_SERVICE, ISiteDiagramService } from './siteDiagram.interface';

@ApiTags('site-diagram')
@ApiSecurity('bearerAuth')
@ApiDefaultResponse({ type: DefaultApiError })
@Controller('site-diagram')
export class SiteDiagramController {
  constructor(
    @Inject(SITE_DIAGRAM_SERVICE)
    private readonly siteDiagramService: ISiteDiagramService,
  ) {}
  @Get('fleet-site-ids')
  async getSiteDiagram(): Promise<string[]> {
    console.log('in get');
    return await this.siteDiagramService.getFleetSiteIds();
  }
}
