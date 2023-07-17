import { ApiProperty } from '@nestjs/swagger';

export const LEGACY_REST_PUT_RESPONSE_DESCRIPTION = `Refer to documentation of target uri / resource.`;

export class LegacyRestPutResponse {
  @ApiProperty({ description: 'HTTP status code', required: false })
  status?: number;
  @ApiProperty({ description: 'Status desciption', required: false })
  statusString?: string;
  @ApiProperty({ description: 'HTTP method', required: false })
  method?: string;
  @ApiProperty({ description: 'Target URI', required: false })
  uri?: string;
  @ApiProperty({ description: 'Value set', required: false })
  value?: any;
  @ApiProperty({ description: 'Information about message request or reply.', required: false })
  message?: string;
}
