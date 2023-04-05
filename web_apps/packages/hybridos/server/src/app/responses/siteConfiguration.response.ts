import { ApiProperty } from '@nestjs/swagger'
import { AppDescriptions } from '../app.constants'
import { SiteConfiguration } from '../app.interface'

export class SiteConfigurationResponse {
    @ApiProperty({description: AppDescriptions.siteConfigurationResponse})
    siteConfiguration: SiteConfiguration
}
