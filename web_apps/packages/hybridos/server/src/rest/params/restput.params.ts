import { ApiProperty } from '@nestjs/swagger';
import { IsNotEmpty } from 'class-validator';

export class RestPutParams {
  @ApiProperty({ description: 'URI' })
  @IsNotEmpty()
  endpoint: string;
}
