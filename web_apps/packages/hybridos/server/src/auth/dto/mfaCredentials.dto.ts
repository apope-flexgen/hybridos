import { ApiProperty } from '@nestjs/swagger';
import { IsNotEmpty, IsNumberString, Length } from 'class-validator';

import { UserDescriptions } from '../../users/users.constants';
import { IsValidUsername } from '../../users/validators/IsValidUsername';
import { AuthDescriptions } from '../auth.constants';

export class MfaCredentials {
  @ApiProperty({ description: UserDescriptions.username })
  @IsNotEmpty()
  @IsValidUsername()
  username: string;
  @ApiProperty({ description: AuthDescriptions.totpCode })
  @IsNumberString()
  @Length(6, 6)
  totp: string;
}
