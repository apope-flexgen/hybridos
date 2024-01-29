import { LoginInfo, SiteConfiguration } from 'src/webuiconfig/webUIConfig.interface';

export const APP_ENV_SERVICE = 'AppEnvService';

export interface IAppEnvService {
  getMongoUri(): string;
  getMongoName(): string;
  getMongoURL(): string;
  getThrottleTTL(): number;
  getThrottleLimit(): number;
  getHttpTimeout(): number;
  getAccessTokenSecretFimsSocket(): string;
  getRefreshTokenTimeout(): number;
  getAccessTokenTimeout(): number;
  getAppServerPort(): number;
  readAggregatedEndpoints();
  getLoginInfo(): LoginInfo;
  getJwtSecretKey(): string;
  getJwtSecretKeyMFA(): string;
  getJwtSecretKeyPasswordExp(): string;
  getSiteConfiguration(): SiteConfiguration;
}
