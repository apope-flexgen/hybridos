// TODO: fix lint
/* eslint-disable @typescript-eslint/no-this-alias */
/* eslint-disable no-constructor-return */
// FIXME: this should not be here
const URL = 'https://172.16.1.80:3001'
let instance: SseService | null = null
class SseService {
  currentEvtSource: EventSource | null = null;

  constructor() {
    if (instance) {
      return instance;
    }
    instance = this;
    return instance;
  }

  listen = (listenRoute: string, fn: (e: MessageEvent) => void): boolean => {
    if (this.currentEvtSource) {
      this.currentEvtSource.close();
    }

    this.currentEvtSource = new EventSource(`${URL}/api/${listenRoute}`, {
      withCredentials: true,
    });
    this.currentEvtSource.onmessage = (message) => {
      fn(message);
    };
    return true;
  };
}

export default new SseService();
