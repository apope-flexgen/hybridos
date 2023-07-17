import { ApiProperty } from '@nestjs/swagger';
import { IsNotEmpty } from 'class-validator';

export class LegacyRestPutParams {
  @ApiProperty({ description: 'Naked Value', required: true })
  @IsNotEmpty()
  value: string;
  @ApiProperty({ description: 'URI' })
  @IsNotEmpty()
  endpoint: string;
}
