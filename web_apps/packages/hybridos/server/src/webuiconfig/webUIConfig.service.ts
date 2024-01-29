import { Inject, Injectable } from '@nestjs/common';
import { SiteConfiguration, LoginInfo } from './webUIConfig.interface';
import { APP_ENV_SERVICE, IAppEnvService } from 'src/environment/appEnv.interface';

@Injectable()
export class WebUIConfigService {
  constructor(
    @Inject(APP_ENV_SERVICE)
    private readonly appEnv: IAppEnvService,
  ) {}
  getSiteConfiguration(): SiteConfiguration | string {
    try {
      const siteConfiguration = this.appEnv.getSiteConfiguration();
      return siteConfiguration;
    } catch (e) {
      return 'routes config undefined';
    }
  }
  getLoginInfo(): LoginInfo | string {
    try {
      const loginInfo = this.appEnv.getLoginInfo();
      return loginInfo;
    } catch (e) {
      return 'login information config undefined';
    }
  }
}
