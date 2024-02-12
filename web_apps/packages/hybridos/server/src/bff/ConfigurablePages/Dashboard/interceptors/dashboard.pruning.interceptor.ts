import { Injectable, ExecutionContext, CallHandler, Inject } from '@nestjs/common';

import { Observable } from 'rxjs';
import { ConfigurablePageDTO } from 'shared/types/dtos/configurablePages.dto';
import { DataPruningInterceptor } from 'src/dataPruning/dataPruning.interceptor';
import { DATA_PRUNING_SERVICE, IDataPruningService } from 'src/dataPruning/dataPruning.interface';
import { SocketIdentifier } from 'src/dataPruning/dataPruning.types';

@Injectable()
export class DashboardPruningInterceptor extends DataPruningInterceptor {
  constructor(@Inject(DATA_PRUNING_SERVICE) dataPruningService: IDataPruningService) {
    super(dataPruningService);
  }

  intercept(context: ExecutionContext, next: CallHandler): Observable<any> {
    return super.intercept(context, next);
  }

  filter = (socketIdentifier: SocketIdentifier, data: ConfigurablePageDTO): boolean => {
    const key = Object.keys(data.displayGroups)[0];

    return this.dataPruningService.filterData(socketIdentifier, key, data);
  };
}
