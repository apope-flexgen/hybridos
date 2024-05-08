import { ApiProperty } from '@nestjs/swagger';
import { IsNotEmpty } from 'class-validator';
import { ErcotOverrideDescriptions } from '../ercotOverride.constants';

export class NewValue {
  @ApiProperty({ description: ErcotOverrideDescriptions.dataValue })
  @IsNotEmpty()
  value: number | boolean;
}
