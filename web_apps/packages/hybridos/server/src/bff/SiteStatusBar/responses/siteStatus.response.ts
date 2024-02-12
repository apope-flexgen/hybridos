import { ApiProperty } from '@nestjs/swagger';
import { IsNumber, IsObject, IsOptional, IsString } from 'class-validator';
import { SiteStatusDescriptions } from '../siteStatus.constants';

export class SiteStatusDataPointInfo {
  @ApiProperty({ description: SiteStatusDescriptions.dataPointLabel })
  @IsString()
  label: string;
  @ApiProperty({ description: SiteStatusDescriptions.dataPointValue })
  @IsString()
  value: string;
  @ApiProperty({ description: SiteStatusDescriptions.dataPointUnit })
  @IsString()
  unit: string;
}

export class SiteStatusDataPointsDTO {
  [uri: string]: SiteStatusDataPointInfo;
}

export class SiteStatusBaseDataDTO {
  @ApiProperty({ description: SiteStatusDescriptions.siteName })
  @IsString()
  @IsOptional()
  siteName?: string;
  @ApiProperty({ description: SiteStatusDescriptions.activeFaults })
  @IsNumber()
  @IsOptional()
  activeFaults?: number;
  @ApiProperty({ description: SiteStatusDescriptions.activeAlarms })
  @IsNumber()
  @IsOptional()
  activeAlarms?: number;
  @ApiProperty({ description: SiteStatusDescriptions.siteState })
  @IsString()
  @IsOptional()
  siteState?: string;
}

export class SiteStatusDataObject {
  @ApiProperty({ description: SiteStatusDescriptions.baseData })
  @IsObject()
  @IsOptional()
  baseData?: SiteStatusBaseDataDTO;

  @ApiProperty({ description: SiteStatusDescriptions.dataPoints })
  @IsObject()
  @IsOptional()
  dataPoints?: SiteStatusDataPointsDTO;
}

export class SiteStatusResponse {
  @ApiProperty({ description: SiteStatusDescriptions.data })
  @IsObject()
  data: SiteStatusDataObject;
}
