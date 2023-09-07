import { ApiProperty } from '@nestjs/swagger';
import { IsString, IsNotEmpty } from 'class-validator';
import { FIMS_URI } from '../api.constants';

export class GetDTO {
  @ApiProperty({ description: FIMS_URI, required: true})
  @IsNotEmpty()
  @IsString()
  uri: string;

  @ApiProperty({ description: FIMS_URI, required: true})
  @IsNotEmpty()
  @IsString()
  replyto: string;
}