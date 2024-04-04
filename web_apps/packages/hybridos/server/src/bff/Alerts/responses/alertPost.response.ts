import { ApiProperty } from '@nestjs/swagger'
import { IsBoolean, IsOptional } from 'class-validator'
import { AlertsDescriptions } from '../alerts.constants'

export class AlertsPostResponse {
    @ApiProperty({ description: AlertsDescriptions.success })
    @IsBoolean()
    success: boolean
    @ApiProperty({ description: AlertsDescriptions.message})
    @IsOptional()
    message?: string
}
