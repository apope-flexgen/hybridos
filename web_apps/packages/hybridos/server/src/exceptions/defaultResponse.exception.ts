import { ApiProperty } from '@nestjs/swagger';

export class DefaultApiError {
  @ApiProperty({ description: 'HTTP status code' })
  statusCode?: number;
  @ApiProperty({ description: 'Message attached to error.' })
  message?: string;
  @ApiProperty({ description: 'Error description' })
  error?: string;
}
