import { Inject, Injectable } from '@nestjs/common';
import { validateOrReject } from 'class-validator';
import * as fs from 'fs';
import { LoginInfo, SiteConfiguration } from 'src/webuiconfig/webUIConfig.interface';
import { ValidationErrors } from '../exceptions/validationErrors.exception';
import { AppConfig } from './AppConfig';
import * as appConsts from './appEnv.constants';
import { IAppEnvService } from 'src/environment/appEnv.interface';

@Injectable()
export class AppEnvService implements IAppEnvService {
  appConfig: AppConfig;
  constructor(
    @Inject('WEB_UI_CONFIG_PATH')
    private webUiConfigPath: string,
    @Inject('WEB_SERVER_CONFIG_PATH')
    private webServerConfigPath: string,
  ) {
    const { serverDataParsed, uiDataParsed } = this.load();
    this.appConfig = new AppConfig(serverDataParsed, uiDataParsed);
    validateOrReject(this.appConfig).catch((errors) => {
      throw new ValidationErrors(errors);
    });
  }

  getMongoUri(): string {
    return `${process.env.MONGO_URL}/${process.env.MONGO_DB_NAME}`;
  }

  getMongoName(): string {
    return process.env.MONGO_DB_NAME;
  }

  getMongoURL(): string {
    return process.env.MONGO_URL;
  }

  getThrottleTTL(): number {
    return appConsts.APP_THROTTLE_CONFIG.default_throttle.ttl;
  }

  getThrottleLimit(): number {
    return appConsts.APP_THROTTLE_CONFIG.default_throttle.limit;
  }

  getHttpTimeout(): number {
    return Number(process.env.HTTP_TIMEOUT);
  }

  getAccessTokenSecretFimsSocket(): string {
    return process.env.ACCESS_TOKEN_SECRET_FIMS_SOCKET;
  }

  getRefreshTokenTimeout(): number {
    return Number(process.env.REFRESH_TOKEN_TIMEOUT);
  }

  getAccessTokenTimeout(): number {
    return Number(process.env.ACCESS_TOKEN_TIMEOUT);
  }

  getAppServerPort(): number {
    return Number(process.env.APP_SERVER_PORT);
  }

  readAggregatedEndpoints() {
    return this.appConfig.aggregatedEndpoints || {};
  }

  getLoginInfo(): LoginInfo {
    const { product, site_name, customer } = this.appConfig;
    return {
      product,
      siteName: site_name !== undefined ? site_name : '',
      customer: customer !== undefined && customer.name !== undefined ? customer.name : '',
      server: customer !== undefined && customer.server !== undefined ? customer.server : '',
    };
  }

  getJwtSecretKey(): string {
    return process.env.JWT_SECRET_KEY;
  }

  getJwtSecretKeyMFA(): string {
    return process.env.JWT_SECRET_KEY_MFA;
  }

  getJwtSecretKeyPasswordExp(): string {
    return process.env.JWT_SECRET_KEY_PASSWORD_EXPIRATION;
  }

  getSiteConfiguration(): SiteConfiguration {
    const {
      timezone,
      product,
      ess,
      gen,
      solar,
      met_station,
      site_status_bar,
      tracker,
      feeders,
      features,
      site,
      events,
      fleet_manager_dashboard,
      control_cabinet,
      scheduler,
      units,
      inspectorComponentsName,
      site_name,
      fleet_name,
      customer,
      system_status,
    } = this.appConfig;

    return {
      timezone,
      product,
      ess,
      gen,
      solar,
      met_station,
      site_status_bar,
      tracker,
      feeders,
      features,
      site,
      events,
      fleet_manager_dashboard,
      control_cabinet,
      scheduler,
      units,
      inspectorComponentsName,
      site_name,
      fleet_name,
      customer,
      system_status,
    };
  }

  private load() {
    const serverData = fs.readFileSync(this.webServerConfigPath, 'utf8');
    const serverDataParsed = JSON.parse(serverData);
    const uiData = fs.readFileSync(this.webUiConfigPath, 'utf8');
    const uiDataParsed = JSON.parse(uiData);
    return { serverDataParsed, uiDataParsed };
  }
}
