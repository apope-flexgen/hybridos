import { ApiProperty } from '@nestjs/swagger';
import { IsString, IsNotEmpty, IsOptional } from 'class-validator';
import { FimsDescriptions } from '../fims.constants';

export class GetDTO {
  @ApiProperty({ description: FimsDescriptions.getURI })
  @IsNotEmpty()
  @IsString()
  uri: string;
  @ApiProperty({ description: FimsDescriptions.bodyReplyTo })
  @IsNotEmpty()
  @IsString()
  replyto: string;
}

export class BodyDTO {
  @ApiProperty({ description: FimsDescriptions.bodyURI })
  @IsNotEmpty()
  @IsString()
  uri: string;
  @ApiProperty({ description: FimsDescriptions.bodyReplyTo })
  @IsNotEmpty()
  @IsString()
  replyto: string;
  @ApiProperty({ description: FimsDescriptions.optionalBody })
  @IsOptional()
  body: string;
}

export class FimsMsgDTO {
  @IsNotEmpty()
  @IsString()
  method: string;
  @IsNotEmpty()
  @IsString()
  uri: string;
  @IsNotEmpty()
  @IsString()
  replyto: string;
  // body = '';
  body: any;
  username = '';
}
