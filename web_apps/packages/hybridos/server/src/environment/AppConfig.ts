import {
    IsInt,
    IsNotEmpty,
    IsNumber,
    IsString,
    IsObject,
    IsBoolean,
    Max,
    Allow,
    IsOptional,
} from 'class-validator'

export class AppConfig {
    @IsNumber()
    APP_SERVER_PORT: number

    @IsNotEmpty()
    @IsString()
    MONGO_URL: string

    @IsNotEmpty()
    @IsString()
    MONGO_DB_NAME: string

    @IsNotEmpty()
    @IsInt()
    HTTP_TIMEOUT: number // milliseconds

    @IsNotEmpty()
    @IsString()
    ACCESS_TOKEN_SECRET_FIMS_SOCKET: string

    @IsNotEmpty()
    @IsNumber()
    @Max(28800) // equals 8hr
    REFRESH_TOKEN_TIMEOUT: number // seconds

    @IsNotEmpty()
    @IsNumber()
    @Max(1800) // equals 30 min
    ACCESS_TOKEN_TIMEOUT: number // seconds

    @Allow()
    aggregatedEndpoints?: any

    @IsNotEmpty()
    @IsString()
    timezone: string

    @IsNotEmpty()
    @IsBoolean()
    ess: boolean

    @IsNotEmpty()
    @IsBoolean()
    gen: boolean

    @IsNotEmpty()
    @IsBoolean()
    solar: boolean

    @IsNotEmpty()
    @IsBoolean()
    met_station: boolean

    @IsNotEmpty()
    @IsBoolean()
    tracker: boolean

    @IsNotEmpty()
    @IsBoolean()
    feeders: boolean

    @IsNotEmpty()
    @IsBoolean()
    features: boolean

    @IsNotEmpty()
    @IsBoolean()
    site: boolean

    @IsNotEmpty()
    @IsBoolean()
    events: boolean

    @IsNotEmpty()
    @IsBoolean()
    control_cabinet: boolean

    @IsNotEmpty()
    @IsBoolean()
    fleet_manager_dashboard: boolean

    @IsNotEmpty()
    @IsBoolean()
    scheduler: boolean

    @IsNotEmpty()
    @IsString()
    product: string

    @IsNotEmpty()
    @IsObject()
    units: {
        [key: string]: string
    }

    @IsNotEmpty()
    @IsString()
    inspectorComponentsName: string

    @IsOptional()
    @IsString()
    site_name: string

    @IsOptional()
    @IsString()
    fleet_name: string

    constructor(
        {
            APP_SERVER_PORT,
            MONGO_URL,
            MONGO_DB_NAME,
            HTTP_TIMEOUT,
            ACCESS_TOKEN_SECRET_FIMS_SOCKET,
            ACCESS_TOKEN_TIMEOUT,
            REFRESH_TOKEN_TIMEOUT,
            aggregatedEndpoints,
        }: {
            APP_SERVER_PORT: number
            MONGO_URL: string
            MONGO_DB_NAME: string
            HTTP_TIMEOUT: number
            ACCESS_TOKEN_SECRET_FIMS_SOCKET: string
            ACCESS_TOKEN_TIMEOUT: number
            REFRESH_TOKEN_TIMEOUT: number
            aggregatedEndpoints: any
        },
        {
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
            control_cabinet,
            fleet_manager_dashboard,
            scheduler,
            units,
            inspectorComponentsName,
            site_name,
            fleet_name,
        }: {
            timezone: string
            product: string
            ess: boolean
            gen: boolean
            solar: boolean
            met_station: boolean
            tracker: boolean
            feeders: boolean
            features: boolean
            site: boolean
            events: boolean
            control_cabinet: boolean
            fleet_manager_dashboard: boolean
            scheduler: boolean
            units: {
                [key: string]: string
            }
            inspectorComponentsName: string
            site_name: string
            fleet_name: string
        }
    ) {
        this.APP_SERVER_PORT = APP_SERVER_PORT
        this.MONGO_URL = MONGO_URL
        this.MONGO_DB_NAME = MONGO_DB_NAME
        this.HTTP_TIMEOUT = HTTP_TIMEOUT
        this.ACCESS_TOKEN_SECRET_FIMS_SOCKET = ACCESS_TOKEN_SECRET_FIMS_SOCKET
        this.REFRESH_TOKEN_TIMEOUT = REFRESH_TOKEN_TIMEOUT
        this.ACCESS_TOKEN_TIMEOUT = ACCESS_TOKEN_TIMEOUT
        this.aggregatedEndpoints = aggregatedEndpoints
        ;(this.timezone = timezone),
            (this.product = product),
            (this.ess = ess),
            (this.gen = gen),
            (this.solar = solar),
            (this.met_station = met_station),
            (this.tracker = tracker),
            (this.feeders = feeders),
            (this.features = features),
            (this.site = site),
            (this.events = events),
            (this.control_cabinet = control_cabinet),
            (this.fleet_manager_dashboard = fleet_manager_dashboard),
            (this.scheduler = scheduler),
            (this.units = units),
            (this.inspectorComponentsName = inspectorComponentsName),
            (this.site_name = site_name),
            (this.fleet_name = fleet_name)
    }
}
