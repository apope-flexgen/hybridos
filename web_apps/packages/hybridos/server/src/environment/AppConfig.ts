import {
    IsNotEmpty,
    IsString,
    IsObject,
    IsBoolean,
    Allow,
    IsOptional,
} from 'class-validator'

export class AppConfig {
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

    @IsOptional()
    @IsBoolean()
    system_status: boolean

    @IsNotEmpty()
    @IsBoolean()
    events: boolean

    @IsNotEmpty()
    @IsBoolean()
    control_cabinet: boolean

    @IsOptional()
    @IsBoolean()
    site_status_bar: boolean

    @IsNotEmpty()
    @IsBoolean()
    fleet_manager_dashboard: boolean

    @IsNotEmpty()
    @IsBoolean()
    scheduler: boolean

    @IsNotEmpty()
    @IsString()
    product: string

    @IsOptional()
    @IsBoolean()
    alerting: boolean

    @IsOptional()
    @IsBoolean()
    site_diagram: boolean

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

    @IsOptional()
    @IsObject()
    customer: {
        name?: string,
        server?: string
    }

    constructor(
        {
            aggregatedEndpoints,
        }: {
            aggregatedEndpoints: any
        },
        {
            timezone,
            product,
            ess,
            gen,
            solar,
            met_station,
            site_status_bar,
            alerting,
            site_diagram,
            tracker,
            feeders,
            features,
            site,
            events,
            control_cabinet,
            fleet_manager_dashboard,
            system_status,
            scheduler,
            units,
            inspectorComponentsName,
            site_name,
            fleet_name,
            customer,
        }: {
            timezone: string
            product: string
            ess: boolean
            gen: boolean
            solar: boolean
            met_station: boolean
            site_status_bar: boolean
            alerting: boolean
            site_diagram: boolean
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
            system_status?: boolean,
            customer: {
                name?: string,
                server?: string
            }
        }
    ) {
        this.aggregatedEndpoints = aggregatedEndpoints
        ;(this.timezone = timezone),
            (this.product = product),
            (this.ess = ess ?? false),
            (this.gen = gen ?? false),
            (this.solar = solar ?? false),
            (this.met_station = met_station ?? false),
            (this.site_status_bar = site_status_bar ?? false),
            (this.alerting = alerting ?? false),
            (this.site_diagram = site_diagram ?? false),
            (this.tracker = tracker ?? false),
            (this.feeders = feeders ?? false),
            (this.features = features ?? false),
            (this.site = site ?? false),
            (this.events = events ?? false),
            (this.control_cabinet = control_cabinet ?? false),
            (this.fleet_manager_dashboard = fleet_manager_dashboard  ?? false),
            (this.scheduler = scheduler ?? false),
            (this.units = units),
            (this.inspectorComponentsName = inspectorComponentsName ?? 'test_controller'),
            (this.site_name = site_name),
            (this.fleet_name = fleet_name),
            (this.system_status = system_status),
            (this.customer = customer)
    }
}
