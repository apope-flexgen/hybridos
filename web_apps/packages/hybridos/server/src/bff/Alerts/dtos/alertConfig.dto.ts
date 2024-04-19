import { ApiProperty } from '@nestjs/swagger'
import {  IsOptional } from 'class-validator'
import { AlertsDescriptions } from '../alerts.constants'
import { Alias, Deadline, Expression, Template } from '../responses/alertConfig.response'

export class AlertConfigDTO {
    @ApiProperty({ description: AlertsDescriptions.alertId })
    @IsOptional()
    id?: string
    @ApiProperty({ description: AlertsDescriptions.deliver })
    @IsOptional()
    enabled?: boolean
    @ApiProperty({ description: AlertsDescriptions.deleted })
    @IsOptional()
    deleted?: boolean
    @ApiProperty({ description: AlertsDescriptions.name })
    @IsOptional()
    title?: string
    @ApiProperty({ description: AlertsDescriptions.deadline })
    @IsOptional()
    deadline?: Deadline
    @ApiProperty({ description: AlertsDescriptions.severity })
    @IsOptional()
    severity?: number
    @ApiProperty({ description: AlertsDescriptions.organization })
    @IsOptional()
    organization?: string
    @ApiProperty({ description: AlertsDescriptions.sites })
    @IsOptional()
    sites?: string[]
    @ApiProperty({ description: AlertsDescriptions.conditions })
    @IsOptional()
    conditions?: Expression[]
    @ApiProperty({ description: AlertsDescriptions.aliases })
    @IsOptional()
    aliases?: Alias[]
    @ApiProperty({ description: AlertsDescriptions.templates })
    @IsOptional()
    templates?: Template[]
}