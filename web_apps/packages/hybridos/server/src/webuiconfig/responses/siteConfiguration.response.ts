import { ApiProperty } from '@nestjs/swagger'
import { WebUIConfigDescriptions, SiteConfigDescriptions } from '../webUIConfig.constants'
import { SiteConfiguration } from '../webUIConfig.interface'
import { IsOptional } from 'class-validator';

export class SiteConfigurationDetailedResponse {
    @ApiProperty({description: SiteConfigDescriptions.timezone})
    @IsOptional()
    timezone: string
    @ApiProperty({description: SiteConfigDescriptions.product})
    @IsOptional()
    product: string
    @ApiProperty({description: SiteConfigDescriptions.ess})
    @IsOptional()
    ess: boolean
    @ApiProperty({description: SiteConfigDescriptions.gen})
    @IsOptional()
    gen: boolean
    @ApiProperty({description: SiteConfigDescriptions.solar})
    @IsOptional()
    solar: boolean
    @ApiProperty({description: SiteConfigDescriptions.met_station})
    @IsOptional()
    met_station: boolean
    @ApiProperty({description: SiteConfigDescriptions.tracker})
    @IsOptional()
    tracker: boolean
    @ApiProperty({description: SiteConfigDescriptions.feeders})
    @IsOptional()
    feeders: boolean
    @ApiProperty({description: SiteConfigDescriptions.features})
    @IsOptional()
    features: boolean
    @ApiProperty({description: SiteConfigDescriptions.site})
    @IsOptional()
    site: boolean
    @ApiProperty({description: SiteConfigDescriptions.events})
    @IsOptional()
    events: boolean
    @ApiProperty({description: SiteConfigDescriptions.control_cabinet})
    @IsOptional()
    control_cabinet: boolean
    @ApiProperty({description: SiteConfigDescriptions.fleet_manager_dashboard})
    @IsOptional()
    fleet_manager_dashboard: boolean
    @ApiProperty({description: SiteConfigDescriptions.scheduler})
    @IsOptional()
    scheduler: boolean
    @ApiProperty({description: SiteConfigDescriptions.units})
    @IsOptional()
    units: object
    @ApiProperty({description: SiteConfigDescriptions.inspectorComponentsName})
    @IsOptional()
    inspectorComponentsName: string
    @ApiProperty({description: SiteConfigDescriptions.site_name})
    @IsOptional()
    site_name: string
    @ApiProperty({description: SiteConfigDescriptions.fleet_name})
    @IsOptional()
    fleet_name: string
    @ApiProperty({description: SiteConfigDescriptions.site_status_bar})
    @IsOptional()
    site_status_bar: boolean
    @ApiProperty({description: SiteConfigDescriptions.customer})
    @IsOptional()
    customer: object
}

export class SiteConfigurationResponse {
    @ApiProperty({
        description: WebUIConfigDescriptions.siteConfigurationResponse,
    })
    siteConfiguration: SiteConfigurationDetailedResponse
}
