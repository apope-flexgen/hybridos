import { Injectable } from '@nestjs/common';
import { SiteConfiguration, LoginInfo } from './webUIConfig.interface';
import { AppEnvService } from '../environment/appEnv.service';

@Injectable()
export class WebUIConfigService {
    constructor(
        private readonly appEnv: AppEnvService,
    ) { }
    getSiteConfiguration(): SiteConfiguration | string {
        try {
            const siteConfiguration = this.appEnv.getSiteConfiguration()
            return siteConfiguration
        } catch (e) {
            return 'routes config undefined'
        }
    }
    getLoginInfo(): LoginInfo | string {
        try {
            const loginInfo = this.appEnv.getLoginInfo()
            return loginInfo
        } catch (e) {
            return 'login information config undefined'
        }
    }
}
