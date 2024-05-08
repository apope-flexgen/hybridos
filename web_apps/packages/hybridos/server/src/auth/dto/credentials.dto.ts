import { ApiProperty } from '@nestjs/swagger';
import { IsAscii, IsNotEmpty } from 'class-validator';
import { UserDescriptions } from '../../users/users.constants';
import { User } from '../../../../shared/types/dtos/auth.dto';

export class Credentials {
  @ApiProperty({ description: UserDescriptions.username })
  @IsNotEmpty()
  @IsAscii()
  username: User['username'];
  @ApiProperty({ description: UserDescriptions.password })
  @IsNotEmpty()
  password: User['password'];
}
