import { Injectable, Inject } from '@nestjs/common';
import { DBI_SERVICE, DBI_URIs } from '../../dbi/dbi.interface';
import { IDBIService } from '../../dbi/dbi.interface';
import { LayoutsResponse } from './responses';
import { ILayoutsService } from './layouts.interface';
import { AddLayout } from './dto/layout.dto';
import { Observable } from 'rxjs';
import { LoggingService } from 'src/logging/logging.service';
import { LogText } from 'src/logging/log_text/log-text';

@Injectable()
export class LayoutsService implements ILayoutsService {
  constructor(
    @Inject(DBI_SERVICE)
    private readonly dbiService: IDBIService,
    private readonly loggingService: LoggingService,
  ) {}

  getLayouts = async (): Promise<Observable<LayoutsResponse>> => {
    try {
      return await this.dbiService.getFromDBI(DBI_URIs.UI_Config_Layout);
    } catch (e) {
      const log: LogText = {
        message: e.message,
        stack: e.stack,
        url: 'layouts',
      };
      this.loggingService.warn(log, '');
      return;
    }
  };

  postLayouts = async (data: AddLayout): Promise<LayoutsResponse> =>
    this.dbiService.postToDBI(DBI_URIs.UI_Config_Layout, data);
}
