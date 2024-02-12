import { Injectable, ExecutionContext, CallHandler, Inject } from '@nestjs/common';

import { Observable } from 'rxjs';
import { SiteStatusResponse } from 'src/bff/SiteStatusBar/sitestatus.interface';
import { DataPruningInterceptor } from 'src/dataPruning/dataPruning.interceptor';
import { DATA_PRUNING_SERVICE, IDataPruningService } from 'src/dataPruning/dataPruning.interface';
import { SocketIdentifier } from 'src/dataPruning/dataPruning.types';

@Injectable()
export class SiteStatusPruningInterceptor extends DataPruningInterceptor {
  constructor(@Inject(DATA_PRUNING_SERVICE) dataPruningService: IDataPruningService) {
    super(dataPruningService);
  }

  intercept(context: ExecutionContext, next: CallHandler): Observable<any> {
    return super.intercept(context, next);
  }

  getKey = (data: SiteStatusResponse): string => {
    return data.data?.dataPoints ? Object.keys(data.data.dataPoints)[0] : 'baseData';
  };

  filter = (socketIdentifier: SocketIdentifier, data: SiteStatusResponse): boolean => {
    const key = this.getKey(data);

    return this.dataPruningService.filterData(socketIdentifier, key, data.data);
  };
}
