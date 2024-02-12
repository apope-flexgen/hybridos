import { ExecutionContext, Injectable } from '@nestjs/common';
import { isEqual } from 'lodash';
import { IDataPruningService } from 'src/dataPruning/dataPruning.interface';
import {
  DataPruningMap,
  DataPruningStoredData,
  SocketIdentifier,
} from 'src/dataPruning/dataPruning.types';

@Injectable()
export class DataPruningService implements IDataPruningService {
  private dataPruningMap: DataPruningMap;

  constructor() {
    this.dataPruningMap = {};
  }

  initSocket = (context: ExecutionContext): void => {
    const socket = context.switchToWs().getClient();

    const { socketID, className, handlerName } = this.getSocketIdentifier(context);

    if (!(socketID in this.dataPruningMap)) {
      this.dataPruningMap[socketID] = {};
      socket.on('close', () => {
        delete this.dataPruningMap[socketID];
      });
    }

    if (!(className in this.dataPruningMap[socketID])) {
      this.dataPruningMap[socketID][className] = {};
    }

    if (!(handlerName in this.dataPruningMap[socketID][className])) {
      this.dataPruningMap[socketID][className][handlerName] = {};
    }
  };

  getSocketIdentifier = (context: ExecutionContext): SocketIdentifier => {
    const socket = context.switchToWs().getClient();

    const remoteAddress = socket._socket.remoteAddress;
    const remotePort = socket._socket.remotePort;
    const socketID = `${remoteAddress}:${remotePort}`;

    const className = context.getClass().name;
    const handlerName = context.getHandler().name;

    return { socketID, className, handlerName };
  };

  getExistingData = (socketIdentifier: SocketIdentifier): DataPruningStoredData => {
    const { socketID, className, handlerName } = socketIdentifier;

    return this.dataPruningMap[socketID][className][handlerName];
  };

  filterData = (socketIdentifier: SocketIdentifier, key: string, data: any): boolean => {
    if (this.isDuplicateData(socketIdentifier, key, data)) {
      return false;
    }

    this.setData(socketIdentifier, key, data);
    return true;
  };

  isDuplicateData = (socketIdentifier: SocketIdentifier, key: string, data: any): boolean => {
    const existingData = this.getExistingData(socketIdentifier);

    return existingData[key] && isEqual(existingData[key], data);
  };

  setData = (socketIdentifier: SocketIdentifier, key: string, data: any) => {
    const { socketID, className, handlerName } = socketIdentifier;

    this.dataPruningMap[socketID][className][handlerName] = {
      ...this.dataPruningMap[socketID][className][handlerName],
      [key]: data,
    };
  };
}
