import { ApiProperty } from '@nestjs/swagger';
import { IsString } from 'class-validator';

export class OneTimeAuthResponse {
  @ApiProperty({ description: 'Token for gaining access websocket gateway.' })
  @IsString()
  token: string;
}
