import { AssetsResponse } from 'src/assets/responses';
import { DashboardsResponse } from 'src/dashboards/responses';
import { LayoutsResponse } from 'src/layouts/responses';
import { User } from 'src/users/dtos/user.dto';
import { FimsMsg } from '../fims/interfaces/fims.interface';

export enum URIs {
  UI_Config_Assets = '/ui_config/assets',
  UI_Config_Dashboard = '/ui_config/dashboard',
  UI_Config_Layout = '/ui_config/layout',
  UI_Config_Audit_Log = '/audit/audit_log_'
};

export interface IDBIService {
  getUIConfigAssets(): Promise<AssetsResponse>
  getUIConfigDashboards(): Promise<DashboardsResponse>
  getUIConfigLayouts(): Promise<LayoutsResponse>
  postUIConfigAssets(data: { data: object[] }, user: User): Promise<AssetsResponse>
  postUIConfigDashboards(data: { data: object[] }, user: User): Promise<DashboardsResponse>
  postUIConfigLayouts(data: { data: object[] }, user: User): Promise<LayoutsResponse>
  postUIConfigAuditLog(data: {
    username?: string,
    userrole?: string,
    modified_field?: string,
    modified_value?: boolean | string
  }): Promise<FimsMsg>
}