import { Injectable } from '@nestjs/common'
import { validateOrReject } from 'class-validator'
import * as fs from 'fs'
import { LoginInfo, SiteConfiguration } from 'src/app/app.interface'
import { ValidationErrors } from '../exceptions/validationErrors.exception'
import { AppConfig } from './AppConfig'

@Injectable()
export class AppEnvService {
    appConfig: AppConfig

    constructor() {
        const { serverDataParsed, uiDataParsed } = this.load()
        this.appConfig = new AppConfig(serverDataParsed, uiDataParsed)
        validateOrReject(this.appConfig).catch((errors) => {
            throw new ValidationErrors(errors)
        })
    }

    getMongoUri(): string {
        return `${this.appConfig.MONGO_URL}/${this.appConfig.MONGO_DB_NAME}`
    }

    getMongoName(): string {
        return this.appConfig.MONGO_DB_NAME
    }

    getMongoURL(): string {
        return this.appConfig.MONGO_URL
    }

    getThrottleTTL(): number {
        return 60
    }

    getThrottleLimit(): number {
        return 210
    }

    getHTTPTimeout(): number {
        return this.appConfig.HTTP_TIMEOUT
    }

    getHttpTimeout(): number {
        return this.appConfig.HTTP_TIMEOUT
    }

    getAccessTokenSecretFimsSocket(): string {
        return this.appConfig.ACCESS_TOKEN_SECRET_FIMS_SOCKET
    }

    getRefreshTokenTimeout(): number {
        return this.appConfig.REFRESH_TOKEN_TIMEOUT
    }

    getAccessTokenTimeout(): number {
        return this.appConfig.ACCESS_TOKEN_TIMEOUT
    }

    getAppServerPort(): number {
        return this.appConfig.APP_SERVER_PORT
    }

    getAggregatedEndpoints() {
        return this.appConfig.aggregatedEndpoints
    }

    getLoginInfo(): LoginInfo {
        const { product } = this.appConfig
        return { product }
    }

    getSiteConfiguration(): SiteConfiguration {
        const {
            timezone,
            product,
            ess,
            gen,
            solar,
            met_station,
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
        } = this.appConfig

        return {
            timezone,
            product,
            ess,
            gen,
            solar,
            met_station,
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
        }
    }

    private load() {
        const serverData = fs.readFileSync(CONFIG_PATH, 'utf8')
        const serverDataParsed = JSON.parse(serverData)
        const uiData = fs.readFileSync(WEB_UI_JSON_CONFIG_PATH, 'utf8')
        const uiDataParsed = JSON.parse(uiData)
        return { serverDataParsed, uiDataParsed }
    }
}
