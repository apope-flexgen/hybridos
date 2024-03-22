import { ApiProperty } from '@nestjs/swagger'
import { IsNumberString, IsOptional } from 'class-validator'
import { AlertsDescriptions } from '../alerts.constants'

export class ActiveAlertsRequest {
    @ApiProperty({ description: AlertsDescriptions.severityFilter })
    @IsOptional()
    severity: number
    @ApiProperty({ description: AlertsDescriptions.limit })
    @IsOptional()
    @IsNumberString()
    limit: string
    @ApiProperty({ description: AlertsDescriptions.page })
    @IsOptional()
    @IsNumberString()
    page: string
    @ApiProperty({ description: AlertsDescriptions.order })
    @IsOptional()
    @IsNumberString()
    order: string
    @ApiProperty({ description: AlertsDescriptions.orderBy })
    @IsOptional()
    orderBy: string
    @ApiProperty({ description: AlertsDescriptions.status })
    @IsOptional()
    status: string
    @ApiProperty({ description: AlertsDescriptions.organization })
    @IsOptional()
    organization: string
    @ApiProperty({ description: AlertsDescriptions.site })
    @IsOptional()
    sites: string[]
    @ApiProperty({ description: AlertsDescriptions.resolved })
    @IsOptional()
    resolved: boolean   
}
