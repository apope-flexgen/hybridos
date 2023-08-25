import { ApiProperty } from '@nestjs/swagger';
import { IsArray, IsObject, IsOptional, IsString } from 'class-validator';
import { LayoutsDescriptions } from '../layouts.constants';

export class Info {
  @ApiProperty({ description: LayoutsDescriptions.infoKey })
  @IsString()
  key: string;
  @ApiProperty({ description: LayoutsDescriptions.infoName })
  @IsString()
  name: string;

  @ApiProperty({ description: LayoutsDescriptions.icon })
  @IsString()
  @IsOptional()
  icon?: string;
}

export class Layout {
  @ApiProperty({ description: LayoutsDescriptions.layouts })
  @IsObject()
  info: Info;
}

export class AddLayout {
  @ApiProperty({
    description: LayoutsDescriptions.data,
    type: Layout,
    isArray: true,
  })
  @IsArray()
  data: Layout[];
}
