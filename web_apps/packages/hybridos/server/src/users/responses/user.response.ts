import { ApiProperty } from '@nestjs/swagger';
import { IsString } from 'class-validator';
import { User } from '../../../../shared/types/dtos/auth.dto';
import { UserDescriptions } from '../users.constants';

export class UserResponse {
  @ApiProperty({ description: UserDescriptions.hybridOSId, required: false })
  @IsString()
  id?: User['id'];
  @ApiProperty({ description: UserDescriptions.username, required: false })
  @IsString()
  username?: User['username'];
  @ApiProperty({ description: UserDescriptions.role, required: false })
  role?: User['role'];
}
