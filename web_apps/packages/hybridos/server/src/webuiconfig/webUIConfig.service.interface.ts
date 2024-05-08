import { LoginInfoResponse } from './responses/product.response';
import { SiteConfigurationResponse } from './responses/siteConfiguration.response';

export const UI_CONFIG_SERVICE = 'WebUIConfigService';

export interface IWebUIConfigService {
  getSiteConfiguration(): Promise<SiteConfigurationResponse>;
  getLoginInfo(): Promise<LoginInfoResponse>;
}
