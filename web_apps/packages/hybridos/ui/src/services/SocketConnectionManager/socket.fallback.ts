// TODO: fix lint
/* eslint-disable @typescript-eslint/no-this-alias */
/* eslint-disable class-methods-use-this */
/* eslint-disable no-constructor-return */
// FIXME: not yet implemented
// if the socket is not available, socket calls will be passed here
// instead, and the interface will be replicated via long polling
import ISocketConnectionManager from './ISocketConnectionManager';

let instance: SocketFallbackManager | null = null;

class SocketFallbackManager implements ISocketConnectionManager {
  constructor() {
    if (instance) {
      return instance;
    }

    instance = this;
  }

  send(): Promise<void> {
    return new Promise((_, reject) => {
      reject();
    });
  }

  listen(): Promise<void> {
    return new Promise((_, reject) => {
      reject();
    });
  }
}

export default new SocketFallbackManager();
