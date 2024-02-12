import { Injectable, ExecutionContext, CallHandler, Inject } from '@nestjs/common';

import { Observable } from 'rxjs';
import { SiteStatusResponse } from 'src/bff/SiteStatusBar/sitestatus.interface';
import { ServiceStatusResponse } from 'src/bff/SystemStatus/dto/serviceStatusResponse.dto';
import { DataPruningInterceptor } from 'src/dataPruning/dataPruning.interceptor';
import { DATA_PRUNING_SERVICE, IDataPruningService } from 'src/dataPruning/dataPruning.interface';
import { SocketIdentifier } from 'src/dataPruning/dataPruning.types';

@Injectable()
export class SystemStatusPruningInterceptor extends DataPruningInterceptor {
  constructor(@Inject(DATA_PRUNING_SERVICE) dataPruningService: IDataPruningService) {
    super(dataPruningService);
  }

  intercept(context: ExecutionContext, next: CallHandler): Observable<any> {
    return super.intercept(context, next);
  }

  filter = (socketIdentifier: SocketIdentifier, data: ServiceStatusResponse): boolean => {
    const key = data.serviceName;

    return this.dataPruningService.filterData(socketIdentifier, key, data);
  };
}
