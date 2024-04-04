import { ApiProperty } from '@nestjs/swagger'
import { IsArray, IsBoolean, IsNumber, IsObject, IsOptional, IsString } from 'class-validator'
import { AlertsDescriptions } from '../alerts.constants'

export interface Template {
  type: "list" | "sequential",
  list?: string[],
  to?: number,
  from?: number,
  token: string,
}
export interface Alias {
  alias: string,
  uri: string,
  type: string,
}

export interface Comparator {
  type: 'alias' | 'literal',
  value: string | number | boolean,
  unit?: string,
}
  
export interface Duration {
  value: string | number,
  unit: "minutes" | "seconds" | "hours",
}

export interface Expression {
  index: number,
  connectionOperator: 'and' | 'or' | null,
  operand1: Comparator,
  operator: string,
  operand2: Comparator,
  duration?: Duration
}

export class AlertConfiguration {
  @ApiProperty({ description: AlertsDescriptions.alertId })
  @IsString()
  id: string
  @ApiProperty({ description: AlertsDescriptions.deliver })
  @IsBoolean()
  enabled: boolean
  @ApiProperty({ description: AlertsDescriptions.deleted })
  @IsOptional()
  deleted?: boolean
  @ApiProperty({ description: AlertsDescriptions.name })
  @IsString()
  title: string
  @ApiProperty({ description: AlertsDescriptions.severity })
  @IsNumber()
  severity: number
  @ApiProperty({ description: AlertsDescriptions.organization })
  @IsString()
  organization: string
  @ApiProperty({ description: AlertsDescriptions.sites })
  @IsString()
  sites: string[]
  @ApiProperty({ description: AlertsDescriptions.trigger_time })
  @IsOptional()
  last_trigger_time?: string
  @ApiProperty({ description: AlertsDescriptions.deadline })
  @IsNumber()
  deadline: number
  @ApiProperty({ description: AlertsDescriptions.aliases })
  @IsArray()
  aliases: Alias[]
  @ApiProperty({ description: AlertsDescriptions.templates })
  @IsOptional()
  templates?: Template[]
  @ApiProperty({ description: AlertsDescriptions.conditions })
  @IsObject()
  conditions: Expression[]
}


export class AlertConfigurationsResponse {
  @ApiProperty({ description: AlertsDescriptions.data, type: AlertConfiguration, isArray: true })
  @IsArray()
  data: AlertConfiguration[]
}
