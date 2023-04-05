// TODO: fix lint
/* eslint-disable class-methods-use-this */
/* eslint-disable @typescript-eslint/no-this-alias */
/* eslint-disable no-constructor-return */
import ISocketConnectionManager from './ISocketConnectionManager';
import SocketFallbackManager from './socket.fallback';
import socketService from './socket.service';

let instance: SocketConnectionManager | null = null;

type ListenerFunc = (event: MessageEvent) => void;

class SocketConnectionManager implements ISocketConnectionManager {
  private socketIsAvailable = false;

  // FIXME: paramaterize this
  private socketRetriesMax = 5;

  // socketIsOpening is used to yield other functions until the socket
  // is open or it has been determined that it will not open
  private socketIsOpening = false;

  // waitingForSocket is used to prevent multiple calls to tryOpenSocket
  // from recursion
  private waitingForSocket = false;

  private listeners: ListenerFunc[] = [];

  constructor() {
    if (instance) {
      return instance;
    }

    instance = this;
    this.tryOpenSocket();
  }

  // FIXME: Implement this for fallbacks as well.
  public setAccessToken = (accessToken: string) => {
    socketService.setAccessToken(accessToken);
  };

  private tryOpenSocket = () => {
    if (this.waitingForSocket) {
      return;
    }

    this.waitingForSocket = true;
    this.socketIsOpening = true;

    socketService.openSocket((socketIsOpen) => {
      this.socketIsAvailable = socketIsOpen;
      // TODO: if the socket successfully connects once in this session, but disconnects later,
      // should we bypass retry max and keep trying to reconnect?
      if (!socketIsOpen && this.socketRetriesMax > 0) {
        this.socketRetriesMax -= 1;
        setTimeout(this.tryOpenSocket, 1000);
      } else if (socketIsOpen) {
        this.listeners.forEach((listener) => {
          socketService.listen(listener);
        });
        this.socketIsOpening = false;
      } else {
        this.socketIsOpening = false;
      }

      this.waitingForSocket = false;
    });
  };

  cleanup = async () => {
    this.socketIsAvailable = false;
    this.socketRetriesMax = 5;
    this.listeners = [];
    await this.tryOpenSocket();
  };

  send = async (event: string, data: any, retryCount?: number): Promise<void> => {
    if (this.socketIsOpening && (!retryCount || retryCount < 5)) {
      const newRetryCount = retryCount ? retryCount + 1 : 1;
      setTimeout(() => this.send(event, data, newRetryCount), 1000);
      return;
    }

    if (this.socketIsAvailable) {
      const sent = await socketService.send(event, data);
      if (sent) {
        return;
      }
    }

    SocketFallbackManager.send();
  };

  listen = async (fn: ListenerFunc, retryCount?: number): Promise<void> => {
    if (this.socketIsOpening && (!retryCount || retryCount < 5)) {
      const newRetryCount = retryCount ? retryCount + 1 : 1;
      setTimeout(() => this.listen(fn, newRetryCount), 1000);
      return;
    }

    if (this.socketIsAvailable) {
      const listening = await socketService.listen(fn);
      if (listening) {
        return;
      }
    }

    // TODO: fix lint
    // eslint-disable-next-line consistent-return
    return SocketFallbackManager.listen();
  };
}

export default new SocketConnectionManager();
