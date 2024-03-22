export enum DBI_URIs {
  UI_Config_Assets = '/ui_config/assets',
  UI_Config_Dashboard = '/ui_config/dashboard',
  UI_Config_Layout = '/ui_config/layout',
  SITE_STATUS_BAR = '/ui_config/site_status',
  UI_Config_Site_Diagram = '/ui_config/psm_tree',
  AUDIT_LOG = '/audit/audit_log_',
  SITE_CONTROLLER_ASSETS = '/site_controller/assets',
  LOCK_MODE_STATE = '/lock_mode/lock_mode_state',
}

export const DBI_SERVICE = 'DBIService';

export interface IDBIService {
  getFromDBI(URI: DBI_URIs): Promise<any>;
  postToDBI(URI: string, data: any): Promise<any>;
}
