// TODO: fix lint
/* eslint-disable @typescript-eslint/no-this-alias */
/* eslint-disable no-constructor-return */

import { axiosSocketConnectionManagerInstance } from 'src/services/axios';

// FIXME: Add config file / env var
const URL = process.env.NODE_ENV === 'development' ? '172.16.1.80:3001' : '172.16.1.80:443';
let instance: SocketService | null = null;
class SocketService {
  private socket: WebSocket | null = null;

  private accessToken = '';

  constructor() {
    if (instance) {
      return instance;
    }
    instance = this;
    return instance;
  }

  setAccessToken = (accessToken: string) => {
    this.accessToken = accessToken;
  };

  send = async (event: string, data: object) => {
    if (this.socket) {
      const payload = JSON.stringify({
        event,
        data,
      });

      this.socket.send(payload);
      return true;
    }

    return false;
  };

  listen = (fn: (e: MessageEvent) => void): boolean => {
    if (this.socket) {
      this.socket.addEventListener('message', (message) => {
        fn(message);
      });
      return true;
    }

    return false;
  };

  openSocket = async (callback?: (socketIsOpen: boolean) => void) => {
    if (this.socket) {
      this.socket.close(1000, 'SocketService.openSocket() was called');
    }

    let oneTimeError = false;
    let response: any;
    try {
      // FIXME: probably shouldn't have a hard-coded URL here
      response = await axiosSocketConnectionManagerInstance('/api/fims/one-time-auth', {
        headers: {
          Authorization: `${this.accessToken}`,
          'Content-Type': 'application/json',
        },
        withCredentials: true,
      });
    } catch (e) {
      console.error(e);
      oneTimeError = true;
    }

    if (oneTimeError || response.status !== 200) {
      callback?.(false);
      return;
    }

    const data = await response.data;

    this.socket = new WebSocket(`wss://${URL}/?oneTime=${data.token}`);

    this.socket.onopen = () => {
      callback?.(true);
    };
    this.socket.onclose = (event) => {
      if (event.code !== 1000) {
        callback?.(false);
      }
    };
    // this.socket.onmessage = (event) => {
    //   console.log('SocketService got a message', event)
    // }
    this.socket.onerror = () => {
      callback?.(false);
    };
  };
}

export default new SocketService();
