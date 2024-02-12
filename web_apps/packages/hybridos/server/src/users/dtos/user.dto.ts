import { ApiProperty } from '@nestjs/swagger';
import { IsOptional } from 'class-validator';
import { UserDescriptions } from '../users.constants';
import { IsValidPassword } from '../validators/IsValidPassword';
import { IsValidRole } from '../validators/IsValidRole';
import { IsValidUsername } from '../validators/IsValidUsername';

export class User {
  id?: string;

  @ApiProperty({ description: UserDescriptions.username })
  @IsOptional()
  @IsValidUsername()
  username?: string;

  @ApiProperty({
    description: 'HybridOS user role',
  })
  @ApiProperty({ description: UserDescriptions.role })
  @IsOptional()
  @IsValidRole()
  role?: string;

  @ApiProperty({ description: UserDescriptions.password })
  @IsOptional()
  @IsValidPassword()
  password?: string;

  old_passwords?: Array<string>;

  pwdUpdatedDate?: Date;

  shared_key?: string;

  mfa_enabled?: boolean;

  version?: string;
}
