import { IsArray, IsObject, IsOptional, IsString } from 'class-validator';
import { siteDiagramDescriptions } from '../dashboard.constants';
import { ApiProperty } from '@nestjs/swagger';

export class SiteDiagramResponse {
  @ApiProperty({
    description: siteDiagramDescriptions.tree,
  })
  @IsObject()
  tree: DiagramTree;
}

export class FleetSiteDiagramsResponse {
  @ApiProperty({
    description: siteDiagramDescriptions.fleetDiagram,
  })
  @IsObject()
  data: FleetDiagram;
}

export class SiteDiagramData implements SiteDiagramNode {
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
  children?: SiteDiagramData[];
}

export interface FleetDiagram {
  [siteName: string]: {
    tree: DiagramTree;
  };
}

export interface SiteDiagramNode {
  id?: string;
  children?: SiteDiagramData[];
}

export interface DiagramTree {
  root: SiteDiagramData;
}
