import { IsArray, IsOptional, IsString } from 'class-validator';
import { siteDiagramDescriptions } from '../dashboard.constants';
import { ApiProperty } from '@nestjs/swagger';

export class SiteDiagramResponse implements SiteDiagramNode {
  @ApiProperty({
    description: siteDiagramDescriptions.id,
  })
  @IsString()
  @IsOptional()
  id?: string;

  @ApiProperty({
    description: siteDiagramDescriptions.asset_type,
  })
  @IsString()
  @IsOptional()
  asset_type?: string;

  @ApiProperty({
    description: siteDiagramDescriptions.children,
  })
  @IsArray()
  @IsOptional()
  children?: SiteDiagramResponse[];
}

export interface SiteDiagramNode {
  id?: string;
  children?: SiteDiagramResponse[];
}
