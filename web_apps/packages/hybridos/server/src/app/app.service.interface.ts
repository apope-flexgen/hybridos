import { LoginInfoResponse } from "./responses/product.response";
import { SiteConfigurationResponse } from "./responses/siteConfiguration.response";

export const APP_SERVICE = 'AppService'

export interface IAppService {
  getSiteConfiguration(): Promise<SiteConfigurationResponse>
  getLoginInfo(): Promise<LoginInfoResponse>
}
