import { ApiProperty } from '@nestjs/swagger'
import { IsNumberString, IsOptional } from 'class-validator'
import { AlertsDescriptions } from '../alerts.constants'

export class AlertsRequest {
    @ApiProperty({ description: AlertsDescriptions.severityFilter })
    @IsOptional()
    @IsNumberString()
    severityFilter: string
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
    statusFilter: string
    @ApiProperty({ description: AlertsDescriptions.organization })
    @IsOptional()
    orgFilter: string
    @ApiProperty({ description: AlertsDescriptions.resolved })
    @IsOptional()
    resolvedFilter: boolean   
}
