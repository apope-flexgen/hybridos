import { ApiProperty } from '@nestjs/swagger';

export const REST_POST_RESPONSE_DESCRIPTION = `Refer to documentation of target uri / resource.`;

export class RestPostResponse {
  @ApiProperty({
    description: 'Information about message request or reply.',
    required: false,
  })
  message?: string;
}
