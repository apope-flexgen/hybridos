/* eslint-disable */
// TODO: fix lint
// - the main issue here is no-underscore-dangle, is this a rule we feel strongly about?
import RealTimeService from 'src/services/RealTimeService/realtime.interface';
import SocketConnection from './socket.connection';

type NamespaceToListeners = {
  [namespace: string]: ListenerFunc[];
};

type ListenerFunc = (data: any) => void;

const generatedNamespaces = new Set();
let namespaceToListeners: NamespaceToListeners = {};
const persistentNamespaceToListeners: NamespaceToListeners = {};

const generateNamespace = (): string => {
  const name = Math.random().toString(36).substring(2, 9);
  if (generatedNamespaces.has(name)) {
    return generateNamespace();
  }

  generatedNamespaces.add(name);
  return name;
};

class SocketService implements RealTimeService {
  private static _instance: SocketService;

  private socketConnection: SocketConnection;

  constructor() {
    this.socketConnection = SocketConnection.Instance;
    this.socketConnection.listener = SocketService.listener;
  }

  public static get Instance() {
    return this._instance || (this._instance = new this());
  }

  private static listener = (msg: any) => {
    if (msg.namespace in namespaceToListeners) {
      namespaceToListeners[msg.namespace].forEach((listener) => {
        listener(msg.data);
      });
    }

    if (msg.namespace in persistentNamespaceToListeners) {
      persistentNamespaceToListeners[msg.namespace].forEach((listener) => {
        listener(msg.data);
      });
    }
  };

  public send: (destination: string, data: any, namespace?: string) => Promise<string> = async (
    destination,
    data,
    namespace,
  ) => {
    namespace = namespace || generateNamespace();
    const msg = {
      event: destination,
      data: {
        data,
        namespace,
      },
    };

    this.socketConnection.send(msg);
    return namespace;
  };

  public listen: <T>(callbackFn: (data: T) => void, namespace: string) => void = (
    callbackFn,
    namespace,
  ) => {
    namespaceToListeners[namespace] = namespaceToListeners[namespace] ?? [];
    namespaceToListeners[namespace].push(callbackFn);
  };

  public sendOnEveryOpen: (destination: string, data: any, namespace?: string) => void = (
    destination,
    data,
    namespace,
  ) => {
    namespace = namespace || generateNamespace();
    this.socketConnection.alwaysSendOnOpen({
      event: destination,
      data: {
        data,
        namespace,
      },
    });
  };

  public persistentListen: (callbackFn: <T = any>(data: T) => void, namespace: string) => void = (
    callbackFn,
    namespace,
  ) => {
    if (namespace in persistentNamespaceToListeners) {
      return;
    }
    persistentNamespaceToListeners[namespace] = [];
    persistentNamespaceToListeners[namespace].push(callbackFn);
  };

  public cleanup: () => void = () => {
    generatedNamespaces.clear();
    namespaceToListeners = {};
    this.socketConnection.resetSocket();
  };

  public setAccessToken: (accessToken: string) => void = (accessToken) => {
    this.socketConnection.accessToken = accessToken;
  };
}

export default SocketService;
