import { ApiProperty } from '@nestjs/swagger';
import { IsArray, IsString } from 'class-validator';
import { SchedulerDescriptions } from '../scheduler.constants';

export class Setpoint {
  @ApiProperty({ description: SchedulerDescriptions.setpoint.id })
  @IsString()
  id: string;
  @ApiProperty({ description: SchedulerDescriptions.setpoint.name })
  @IsString()
  name: string;
  @ApiProperty({ description: SchedulerDescriptions.setpoint.type })
  @IsString()
  type: string;
  @ApiProperty({ description: SchedulerDescriptions.setpoint.unit })
  @IsString()
  unit: string;
  @ApiProperty({ description: SchedulerDescriptions.setpoint.uri })
  @IsString()
  uri: string;
  @ApiProperty({ description: SchedulerDescriptions.setpoint.value })
  value: number | boolean;
}

export class ModeBody {
  @ApiProperty({ description: SchedulerDescriptions.mode.name })
  @IsString()
  name: string;
  @ApiProperty({ description: SchedulerDescriptions.mode.colorColode })
  @IsString()
  color_code:
    | 'orange'
    | 'lightGreen'
    | 'teal'
    | 'lightBlue'
    | 'indigo'
    | 'deepPurple'
    | 'red'
    | 'pink'
    | 'deepOrange'
    | 'gray';
  @ApiProperty({ description: SchedulerDescriptions.mode.icon })
  @IsString()
  icon: string;
  @ApiProperty({
    description: SchedulerDescriptions.mode.variables,
    type: Setpoint,
    isArray: true,
  })
  @IsArray()
  variables: Setpoint[];
  @ApiProperty({
    description: SchedulerDescriptions.mode.constants,
    type: Setpoint,
    isArray: true,
  })
  @IsArray()
  constants: Setpoint[];
}
