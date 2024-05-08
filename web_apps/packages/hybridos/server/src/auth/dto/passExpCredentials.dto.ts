import { ApiProperty } from '@nestjs/swagger';
import { IsAscii, IsNotEmpty } from 'class-validator';

import { UserDescriptions } from '../../users/users.constants';
import { IsValidPassword } from '../../users/validators/IsValidPassword';

export class PassExpCredentials {
  @ApiProperty({ description: UserDescriptions.username })
  @IsNotEmpty()
  @IsAscii()
  username: string;
  @ApiProperty({ description: UserDescriptions.password })
  @IsNotEmpty()
  @IsValidPassword()
  updatedPassword: string;
}
