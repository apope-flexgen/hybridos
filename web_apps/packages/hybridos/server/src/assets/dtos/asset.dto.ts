import { ApiProperty } from '@nestjs/swagger';
import { IsArray, IsObject, IsString } from 'class-validator';
import { AssetsDescriptions } from '../assets.constants';

export class Status {
  @ApiProperty({ description: AssetsDescriptions.statusName })
  @IsString()
  name: string;
  @ApiProperty({ description: AssetsDescriptions.statusScalar })
  @IsString()
  scalar: string;
  @ApiProperty({ description: AssetsDescriptions.statusUnits })
  @IsString()
  units: string;
  @ApiProperty({ description: AssetsDescriptions.statusURI })
  @IsString()
  uri: string;
}

export class Control {
  @ApiProperty({ description: AssetsDescriptions.controlInputType })
  @IsString()
  inputType: string;
  @ApiProperty({ description: AssetsDescriptions.controlName })
  @IsString()
  name: string;
  @ApiProperty({ description: AssetsDescriptions.controlScalar })
  @IsString()
  scalar: string;
  @ApiProperty({ description: AssetsDescriptions.controlUnits })
  @IsString()
  units: string;
  @ApiProperty({ description: AssetsDescriptions.controlURI })
  @IsString()
  uri: string;
}

export class Alarm {
  @ApiProperty({ description: AssetsDescriptions.alarmFields, isArray: true })
  @IsArray()
  alarmFields: any[];
  @ApiProperty({ description: AssetsDescriptions.faultFields, isArray: true })
  @IsArray()
  faultFields: any[];
}

export class Asset {
  @ApiProperty({ description: AssetsDescriptions.alarms })
  @IsObject()
  alarms: Alarm;
  @ApiProperty({
    description: AssetsDescriptions.batchControls,
    type: Control,
    isArray: true,
  })
  @IsArray()
  batchControls: Control[];
  @ApiProperty({
    description: AssetsDescriptions.controls,
    type: Control,
    isArray: true,
  })
  @IsArray()
  controls: Control[];
  @ApiProperty({ description: AssetsDescriptions.info })
  @IsObject()
  info: {
    [key: string]: any;
    alarmFields: string[];
    faultFields: string[];
  };
  @ApiProperty({
    description: AssetsDescriptions.statuses,
    type: Status,
    isArray: true,
  })
  @IsArray()
  statuses: Status[];
  @ApiProperty({ description: AssetsDescriptions.summary, isArray: true })
  @IsArray()
  summary: any[];
  @ApiProperty({
    description: AssetsDescriptions.summaryControls,
    isArray: true,
  })
  @IsArray()
  summaryControls: any[];
}

export class Assets {
  @ApiProperty({
    description: AssetsDescriptions.assets,
    type: Asset,
    isArray: true,
  })
  @IsArray()
  data: Asset[];
}
