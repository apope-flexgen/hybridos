/* eslint-disable */
// TODO: fix lint
// - the main issue here is no-underscore-dangle, is this a rule we feel strongly about?
import { axiosSocketConnectionManagerInstance } from 'src/services/axios';

type MsgType = {
  event: string;
  data: any;
};
// FIXME: these shouldn't live here but I don't know where they should go
const ONE_TIME_AUTH_URI = '/api/fims/one-time-auth';
const { SOCKET_URI, SOCKET_PORT } =
  process.env.NODE_ENV === 'dev' || process.env.NODE_ENV === 'development'
    ? { SOCKET_URI: 'wss://172.16.1.90', SOCKET_PORT: '3001' }
    : {
        SOCKET_URI: `wss://${window.location.hostname}`,
        SOCKET_PORT:
          window.location.port && window.location.port !== '443' ? window.location.port : '443',
      };
const MAX_RETRIES = 5;

export default class SocketConnection {
  private static _instance: SocketConnection;

  private _accessToken = '';

  private _socket?: WebSocket;

  private _msgQueue: MsgType[] = [];

  private _sendOnOpen: MsgType[] = [];

  private _listener: ((data: any) => void) | undefined;

  private _isOpening = false;

  private constructor() {
    this.openNewSocket();
  }

  public static get Instance() {
    return this._instance || (this._instance = new this());
  }

  public set accessToken(accessToken: string) {
    this._accessToken = accessToken;
  }

  public set listener(listener: (data: any) => void) {
    this._listener = listener;
  }

  public alwaysSendOnOpen(msg: MsgType) {
    this._sendOnOpen.push(msg);
  }

  public async send(msg: MsgType) {
    if (this._socket === undefined) {
      this._msgQueue.push(msg);
      this.openNewSocket();
      return;
    }

    if (this._socket.readyState === WebSocket.OPEN) {
      this._socket.send(JSON.stringify(msg));
      return;
    }

    if (this._socket.readyState === WebSocket.CONNECTING) {
      this._msgQueue.push(msg);
      return;
    }

    this._msgQueue.push(msg);
    this.openNewSocket();
  }

  public resetSocket = () => {
    if (this._socket) {
      this._socket.close();
    }

    this.openNewSocket();
  };

  private async openNewSocket(retries: number = MAX_RETRIES) {
    if (this._socket !== undefined && this._socket.readyState <= WebSocket.OPEN) {
      return;
    }

    if (this._accessToken === '') {
      return;
    }

    if (this._isOpening && retries === MAX_RETRIES) {
      return;
    }

    this._isOpening = true;

    let hasOpened = false;

    if (retries < 0) {
      console.error('Failed to open socket');
      return;
    }

    let token: string;

    try {
      token = await this.getOneTimeAuthCode();
    } catch (e) {
      console.error(e);
      this.openNewSocket(retries - 1);
      return;
    }

    const newSocket = new WebSocket(`${SOCKET_URI}:${SOCKET_PORT}/?oneTime=${token}`);

    newSocket.onopen = () => {
      this.drainQueue();
      retries = MAX_RETRIES;
      this._isOpening = false;
      hasOpened = true;
    };

    newSocket.onclose = () => {
      this._isOpening = false;
      !hasOpened && this.openNewSocket(retries - 1);
    };

    newSocket.onerror = () => {};

    newSocket.onmessage = (event) => {
      if (event.data !== undefined && this._listener !== undefined) {
        this._listener(JSON.parse(event.data));
      }
    };

    this._socket = newSocket;
  }

  private drainQueue() {
    while (this._msgQueue.length > 0) {
      const msg = this._msgQueue.shift();
      msg !== undefined && this.send(msg);
    }

    // these shouldn't go through the protected send method
    // because they always send on open - if they ended up in the
    // queue, they would be sent twice
    this._sendOnOpen.forEach((msg) => {
      this._socket?.send(JSON.stringify(msg));
    });
  }

  private getOneTimeAuthCode = async (): Promise<string> => {
    let response: any;
    try {
      response = await axiosSocketConnectionManagerInstance(ONE_TIME_AUTH_URI, {
        headers: {
          Authorization: `${this._accessToken}`,
          'Content-Type': 'application/json',
        },
        withCredentials: true,
      });
    } catch (e) {
      throw new Error('Failed to get one-time auth code');
    }

    if (response.status !== 200) {
      throw new Error('Failed to get one-time auth code');
    }

    const { token } = await response.data;
    return token;
  };
}
