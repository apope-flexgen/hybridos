// TODO: fix lint
/* eslint-disable class-methods-use-this */
/* eslint-disable no-constructor-return */
import SocketConnectionManager from 'src/services/SocketConnectionManager';
import SseService from 'src/services/SseService';

const instance: QueryService | null = null;

class QueryService {
  constructor() {
    return instance || this;
  }

  cleanupSocket = async () => {
    SocketConnectionManager.cleanup();
  };

  getConfigurablePage(listenerFunction: (
    e: MessageEvent
  ) => void, pageName: string, pageCategory?: string) {
    if (pageName === 'assetsPage' && pageCategory) {
      this.getAssetsPage(pageCategory, listenerFunction);
      return;
    }
    if (pageName === 'dashboard') {
      this.getDashboard(listenerFunction);
      return;
    }

    console.error('Invalid page name');
  }

  private getAssetsPage: (
    category: string,
    listenerFunction: (e: MessageEvent) => void
  ) => void = async (
      category: string,
      listenerFunction: (e: MessageEvent) => void,
    ) => {
      SocketConnectionManager.listen(listenerFunction);
      SocketConnectionManager.send('assetsPage', category);
    };

  private getDashboard: (listenerFunction: (e: MessageEvent) => void) => void = async (
    listenerFunction: (e: MessageEvent) => void,
  ) => {
    SocketConnectionManager.listen(listenerFunction);
    SocketConnectionManager.send('dashboard', '');
  };

  getSiteStatusBar: (listenerFunction: (e: MessageEvent) => void) => void = async (
    listenerFunction: (e: MessageEvent) => void,
  ) => {
    SseService.listen('sitestatus', listenerFunction);
  };

  // FIXME: this should be combined into generic getPage
  getSchedulerPage: (URIs: string[], listenerFunction: (e: MessageEvent) => void) => void = async (
    URIs: string[],
    listenerFunction: (e: MessageEvent) => void,
  ) => {
    SocketConnectionManager.listen(listenerFunction);
    SocketConnectionManager.send('scheduler', URIs);
  };

  getVariableOverridePage:
  (siteId: string, listenerFunction: (e: MessageEvent) => void) => void = async (
      siteId: string,
      listenerFunction: (e: MessageEvent) => void,
    ) => {
      SocketConnectionManager.listen(listenerFunction);
      SocketConnectionManager.send('variableOverride', siteId);
    };
}

export default new QueryService();
