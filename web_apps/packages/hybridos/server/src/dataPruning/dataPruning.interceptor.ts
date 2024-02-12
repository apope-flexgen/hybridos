import { Injectable, NestInterceptor, ExecutionContext, CallHandler, Inject } from '@nestjs/common';

import { Observable, filter } from 'rxjs';
import { DATA_PRUNING_SERVICE, IDataPruningService } from 'src/dataPruning/dataPruning.interface';
import { SocketIdentifier } from 'src/dataPruning/dataPruning.types';

@Injectable()
export abstract class DataPruningInterceptor implements NestInterceptor {
  // block entire messages
  abstract filter: (socketIdentifier: SocketIdentifier, data: any) => boolean;

  constructor(@Inject(DATA_PRUNING_SERVICE) protected dataPruningService: IDataPruningService) {}

  intercept(context: ExecutionContext, next: CallHandler): Observable<any> {
    const socketIdentifier = this.getSocketIdentifier(context);

    return next.handle().pipe(filter((data) => this.filter(socketIdentifier, data)));
  }

  getSocketIdentifier = (context: ExecutionContext): SocketIdentifier => {
    this.dataPruningService.initSocket(context);

    return this.dataPruningService.getSocketIdentifier(context);
  };
}
