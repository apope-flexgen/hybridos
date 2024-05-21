import { Injectable, ExecutionContext, CallHandler, Inject } from '@nestjs/common';

import { Observable } from 'rxjs';
import { ConfigurablePageDTO } from 'shared/types/dtos/configurablePages.dto';
import { DataPruningInterceptor } from 'src/dataPruning/dataPruning.interceptor';
import { DATA_PRUNING_SERVICE, IDataPruningService } from 'src/dataPruning/dataPruning.interface';
import { SocketIdentifier } from 'src/dataPruning/dataPruning.types';

@Injectable()
export class SiteDiagramPruningInterceptor extends DataPruningInterceptor {
  constructor(@Inject(DATA_PRUNING_SERVICE) dataPruningService: IDataPruningService) {
    super(dataPruningService);
  }

  intercept(context: ExecutionContext, next: CallHandler): Observable<any> {
    return super.intercept(context, next);
  }

  getKey = (data: ConfigurablePageDTO): string => {
    return Object.keys(data.displayGroups)[0];
  };

  filter = (socketIdentifier: SocketIdentifier, data: ConfigurablePageDTO): boolean => {
    if (data.hasStatic) {
      return true;
    }

    const key = this.getKey(data);

    return this.dataPruningService.filterData(socketIdentifier, key, data.displayGroups[key]);
  };
}
