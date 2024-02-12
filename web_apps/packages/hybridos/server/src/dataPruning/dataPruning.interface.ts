import { ExecutionContext } from '@nestjs/common';
import { SocketIdentifier } from 'src/dataPruning/dataPruning.types';

export const DATA_PRUNING_SERVICE = 'DataPruningService';

export interface IDataPruningService {
  initSocket(context: ExecutionContext): void;
  getSocketIdentifier(context: ExecutionContext): SocketIdentifier;
  filterData(socketIdentifier: SocketIdentifier, key: string, data: any): boolean;
}
