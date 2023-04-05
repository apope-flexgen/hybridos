import { Injectable, Inject } from '@nestjs/common'
import { SiteConfiguration, LoginInfo } from './app.interface';
import { AppEnvService } from '../environment/appEnv.service'
import { FimsService } from '../fims/fims.service';
import { FIMS_SERVICE } from '../fims/interfaces/fims.interface';
import { IValidJWTService, VALID_JWT_SERVICE } from 'src/auth/interfaces/validJWT.service.interface';

@Injectable()
export class AppService {
    constructor(
        @Inject(FIMS_SERVICE) private readonly fimsService: FimsService,
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
