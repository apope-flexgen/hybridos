import { ApiProperty } from '@nestjs/swagger';
import { IsArray, IsBoolean, IsNumber, IsOptional, IsString } from 'class-validator';
import { AlertsDescriptions } from '../alerts.constants';

export interface Details {
  message: string;
  timestamp: string;
}

export class ActiveAlert {
  @ApiProperty({ description: AlertsDescriptions.alertId })
  @IsString()
  id: string;
  @ApiProperty({ description: AlertsDescriptions.status })
  @IsString()
  status: string;
  @ApiProperty({ description: AlertsDescriptions.title })
  @IsString()
  title: string;
  @ApiProperty({ description: AlertsDescriptions.severity })
  @IsNumber()
  severity: number;
  @ApiProperty({ description: AlertsDescriptions.organization })
  @IsOptional()
  organization?: string;
  @ApiProperty({ description: AlertsDescriptions.site })
  @IsOptional()
  site?: string;
  @ApiProperty({ description: AlertsDescriptions.resolved })
  @IsBoolean()
  @IsOptional()
  resolved?: boolean;
  @ApiProperty({ description: AlertsDescriptions.resolution_time })
  @IsOptional()
  resolution_time?: string;
  @ApiProperty({ description: AlertsDescriptions.resolution_message })
  @IsOptional()
  resolution_message?: string;
  @ApiProperty({ description: AlertsDescriptions.trigger_time })
  @IsOptional()
  trigger_time?: string;
  @ApiProperty({ description: AlertsDescriptions.details })
  @IsArray()
  details: Details[];
  @ApiProperty({ description: AlertsDescriptions.deadline })
  @IsNumber()
  deadline: number;
}

export class ActiveAlertsResponse {
  @ApiProperty({ description: AlertsDescriptions.count })
  @IsNumber()
  count: number;
  @ApiProperty({ description: AlertsDescriptions.data, type: ActiveAlert, isArray: true })
  @IsArray()
  data: ActiveAlert[];
}

export class ResolveAlertResponse {
  @ApiProperty({ description: AlertsDescriptions.success })
  @IsBoolean()
  success: boolean;
  @ApiProperty({ description: AlertsDescriptions.message })
  @IsString()
  message: string;
}

export class ResolvedAlert {
  @ApiProperty({ description: AlertsDescriptions.alertId })
  @IsString()
  id: string;
  @ApiProperty({ description: AlertsDescriptions.title })
  @IsString()
  title: string;
  @ApiProperty({ description: AlertsDescriptions.severity })
  @IsNumber()
  severity: number;
  @ApiProperty({ description: AlertsDescriptions.organization })
  @IsOptional()
  organization?: string;
  @ApiProperty({ description: AlertsDescriptions.site })
  @IsOptional()
  site?: string;
  @ApiProperty({ description: AlertsDescriptions.resolved })
  @IsBoolean()
  resolved: boolean;
  @ApiProperty({ description: AlertsDescriptions.resolution_time })
  @IsString()
  resolution_time: string;
  @ApiProperty({ description: AlertsDescriptions.resolution_message })
  @IsString()
  resolution_message: string;
  @ApiProperty({ description: AlertsDescriptions.trigger_time })
  @IsString()
  trigger_time: string;
  @ApiProperty({ description: AlertsDescriptions.details })
  @IsArray()
  details: Details[];
  @ApiProperty({ description: AlertsDescriptions.deadline })
  @IsNumber()
  deadline: number;
}

export class ResolvedAlertsResponse {
  @ApiProperty({ description: AlertsDescriptions.count })
  @IsNumber()
  count: number;
  @ApiProperty({
    description: AlertsDescriptions.data,
    type: ResolvedAlert,
    isArray: true,
  })
  @IsArray()
  data: ResolvedAlert[];
}
