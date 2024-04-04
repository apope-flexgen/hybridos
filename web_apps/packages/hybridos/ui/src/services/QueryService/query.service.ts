/* eslint-disable */
// TODO: fix lint
import { DashboardLayout } from 'shared/types/dtos/configurablePages.dto';
import { TableDashboardDataTableDTO } from 'shared/types/dtos/dataTables.dto';
import RealTimeService from 'src/services/RealTimeService/realtime.service';

const instance: QueryService | null = null;

class QueryService {
  realTimeService: RealTimeService = RealTimeService.Instance;

  constructor() {
    return instance || this;
  }

  cleanupSocket = async () => {
    RealTimeService.Instance.cleanup();
  };

  getConfigurablePage(
    listenerFunction: (e: MessageEvent) => void,
    pageName: string,
    assetKey?: string,
  ) {
    if (pageName === 'assetsPage' && assetKey) {
      this.getAssetsPage(assetKey, listenerFunction);
      return;
    }
    if (pageName === 'dashboard') {
      this.getDashboard(listenerFunction);
      return;
    }

    console.error('Invalid page name');
  }

  private getAssetsPage: (assetKey: string, listenerFunction: (data: any) => void) => void = async (
    assetKey: string,
    listenerFunction: (data: any) => void,
  ) => {
    this.realTimeService.listen((data) => {
      listenerFunction(data);
    }, `assetsPage-${assetKey}`);
    this.realTimeService.send('assetsPage', assetKey, `assetsPage-${assetKey}`);
  };

  getTableDashboard = (listenerFunction: (data: TableDashboardDataTableDTO) => void) => {
    this.realTimeService.listen(listenerFunction, 'tableDashboard');
    this.realTimeService.send('dashboard', DashboardLayout.TABLE, 'tableDashboard');
  };

  private getDashboard: (listenerFunction: (data: any) => void) => void = async (
    listenerFunction: (data: any) => void,
  ) => {
    this.realTimeService.listen(listenerFunction, 'cardDashboard');
    this.realTimeService.send('dashboard', DashboardLayout.CARD, 'cardDashboard');
  };

  getSiteStatusBar: (listenerFunction: (data: any) => void) => void = async (
    listenerFunction: (data: any) => void,
  ) => {
    this.realTimeService.sendOnEveryOpen('sitestatus', '', 'sitestatus');
    this.realTimeService.persistentListen(listenerFunction, 'sitestatus');
  };

  getSystemStatus: (listenerFunction: (data: any) => void) => void = async (
    listenerFunction: (data: any) => void,
  ) => {
    this.realTimeService.listen(listenerFunction, 'systemStatus');
    this.realTimeService.send('systemStatus', '', 'systemStatus');
  };

  getSchedulerPage: (URIs: string[], listenerFunction: (data: any) => void) => void = async (
    URIs: string[],
    listenerFunction: (data: any) => void,
  ) => {
    this.realTimeService.listen(listenerFunction, 'scheduler');
    this.realTimeService.send('scheduler', URIs, 'scheduler');
  };

  getAlertsPage: (listenerFunction: (data: any) => void) => void = async (
    listenerFunction: (data: any) => void,
  ) => {
    this.realTimeService.listen(listenerFunction, 'alerts');
    this.realTimeService.send('alerts', '', 'alerts');
  };
  
  getErcotOverridePage: (siteId: string, listenerFunction: (data: any) => void) => void = async (
    siteId: string,
    listenerFunction: (data: any) => void,
  ) => {
    this.realTimeService.listen(listenerFunction, 'ercotOverride');
    this.realTimeService.send('ercotOverride', siteId, 'ercotOverride');
  };

  getLayouts: (listenerFunction: (data: any) => void) => void = async (
    listenerFunction: (data: any) => void,
  ) => {
    this.realTimeService.sendOnEveryOpen('layouts', '', 'layouts');
    this.realTimeService.persistentListen(listenerFunction, 'layouts');
  };

  getWsException: (listenerFunction: (data: any) => void) => void = async (
    listenerFunction: (data: any) => void,
  ) => {
    this.realTimeService.persistentListen(listenerFunction, 'exception');
  };
}

export default new QueryService();
