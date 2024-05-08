import { ApiProperty } from '@nestjs/swagger';
import { AuthDescriptions } from '../auth.constants';
import { LogoutResponse } from '../../../../shared/types/dtos/auth.dto';

export class Logout {
  @ApiProperty({ description: AuthDescriptions.logout })
  message: LogoutResponse['message'];
}
