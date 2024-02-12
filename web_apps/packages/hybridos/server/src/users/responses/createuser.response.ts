import { ApiProperty } from '@nestjs/swagger';
import { IsString } from 'class-validator';
import { User } from '../../../../shared/types/dtos/auth.dto';
import { UserDescriptions } from '../users.constants';

export class CreateUserResponse {
  @ApiProperty({ description: UserDescriptions.hybridOSId, required: false })
  @IsString()
  id?: User['id'];
}
