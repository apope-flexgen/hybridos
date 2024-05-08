import { ApiProperty } from '@nestjs/swagger';
import { IsArray, IsBoolean, IsObject, IsOptional, IsString } from 'class-validator';
import { DashboardsDescriptions } from '../dashboards.constants';

export class Item {
  @ApiProperty({ description: DashboardsDescriptions.infoBaseURI })
  @IsString()
  name: string;
  @ApiProperty({ description: DashboardsDescriptions.infoBaseURI })
  @IsString()
  uri: string;
}

export class Info {
  @ApiProperty({ description: DashboardsDescriptions.infoBaseURI })
  @IsString()
  @IsOptional()
  baseURI?: string;
  @ApiProperty({ description: DashboardsDescriptions.infoBatteryView })
  @IsBoolean()
  @IsOptional()
  batteryView?: boolean;
  @ApiProperty({ description: DashboardsDescriptions.infoBatteryViewSourceURI })
  @IsOptional()
  batteryViewSourceURI?: null | string;
  @ApiProperty({ description: DashboardsDescriptions.batteryViewURI })
  @IsString()
  @IsOptional()
  batteryViewURI?: string;
  @ApiProperty({ description: DashboardsDescriptions.isTemplate })
  @IsBoolean()
  @IsOptional()
  isTemplate?: boolean;
  @ApiProperty({ description: DashboardsDescriptions.items, type: Item, isArray: true })
  @IsArray()
  @IsOptional()
  items?: Item[];
  @ApiProperty({ description: DashboardsDescriptions.name })
  @IsString()
  @IsOptional()
  name?: string;
  @ApiProperty({
    description: DashboardsDescriptions.sourceURIs,
    type: String,
    isArray: true,
  })
  @IsArray()
  @IsOptional()
  sourceURIs?: string[];
}

export class Status {
  @ApiProperty({ description: DashboardsDescriptions.statusName })
  @IsString()
  name: string;
  @ApiProperty({ description: DashboardsDescriptions.statusScalar })
  @IsString()
  scalar: string;
  @ApiProperty({ description: DashboardsDescriptions.statusUnits })
  @IsString()
  units: string;
  @ApiProperty({ description: DashboardsDescriptions.statusURI })
  @IsString()
  uri: string;
}

export class Dashboard {
  @ApiProperty({ description: DashboardsDescriptions.info })
  @IsObject()
  info: Info;
  @ApiProperty({
    description: DashboardsDescriptions.status,
    type: Status,
    isArray: true,
  })
  @IsArray()
  status: Status[];
}

export class DashboardCard {
  @ApiProperty({
    description: DashboardsDescriptions.data,
    type: Dashboard,
    isArray: true,
  })
  @IsArray()
  data: Dashboard[];
}
