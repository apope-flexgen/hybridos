import { Injectable, Inject } from '@nestjs/common'
import { SiteConfiguration, LoginInfo } from './app.interface';
import { AppEnvService } from '../environment/appEnv.service'
import { FimsService } from '../fims/fims.service';

@Injectable()
export class AppService {
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
