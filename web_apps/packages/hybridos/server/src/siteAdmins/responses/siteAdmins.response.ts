import { ApiProperty } from '@nestjs/swagger';
import { Type } from 'class-transformer';
import {
  IsAscii,
  IsBoolean,
  IsInt,
  IsIP,
  IsNotEmpty,
  IsPort,
  IsPositive,
  IsString,
  Max,
  Min,
  ValidateNested,
} from 'class-validator';

import { SiteAdminsDescriptions } from '../siteAdmins.constants';
import { ValidPassExpInterval } from '../validators/passwordExpirationInterval.validator';

class PasswordSettingResponse {
  @ApiProperty({ description: SiteAdminsDescriptions.mfaIsEnabled })
  @IsBoolean()
  multi_factor_authentication: boolean;

  @ApiProperty({ description: SiteAdminsDescriptions.passwordExpiration })
  @IsBoolean()
  password_expiration: boolean;

  @ApiProperty({ description: SiteAdminsDescriptions.passwordMinLength })
  @IsInt()
  @Min(8)
  @Max(96)
  minimum_password_length: number;

  @ApiProperty({ description: SiteAdminsDescriptions.passwordMaxLength })
  @IsInt()
  @Min(16)
  @Max(1028)
  maximum_password_length: number;

  @ApiProperty({ description: SiteAdminsDescriptions.passExpInt })
  @ValidPassExpInterval()
  password_expiration_interval: string;

  @ApiProperty({ description: SiteAdminsDescriptions.passOldPasswords })
  @IsInt()
  @Min(0)
  @Max(50)
  old_passwords: number;

  @ApiProperty({ description: SiteAdminsDescriptions.passRegEx })
  @IsString()
  password_regular_expression: string;

  @ApiProperty({ description: SiteAdminsDescriptions.lowercase })
  @IsBoolean()
  lowercase: boolean;

  @ApiProperty({ description: SiteAdminsDescriptions.uppercase })
  @IsBoolean()
  uppercase: boolean;

  @ApiProperty({ description: SiteAdminsDescriptions.digit })
  @IsBoolean()
  digit: boolean;

  @ApiProperty({ description: SiteAdminsDescriptions.special })
  @IsBoolean()
  special: boolean;
}
class RadiusSettingResponse {
  @ApiProperty({ description: SiteAdminsDescriptions.radiusIsEnabled })
  @IsBoolean()
  is_enabled: boolean;

  @ApiProperty({ description: SiteAdminsDescriptions.radiusIp })
  @IsIP()
  ip_address: string;

  @ApiProperty({ description: SiteAdminsDescriptions.radiusPort })
  @IsPort()
  port: string;

  @ApiProperty({ description: SiteAdminsDescriptions.radiusSecretPhrase })
  @IsAscii()
  @IsNotEmpty()
  secret_phrase: string;

  @ApiProperty({ description: SiteAdminsDescriptions.radiusWaitTime })
  @IsInt()
  @IsPositive()
  wait_time: number;

  @ApiProperty({
    description: SiteAdminsDescriptions.radiusLocalAuthDisabled,
  })
  @IsBoolean()
  is_local_auth_disabled: boolean;
}

export class SiteAdminsResponse {
  @ApiProperty({ description: SiteAdminsDescriptions.passwordSettings })
  @Type(() => PasswordSettingResponse)
  @ValidateNested()
  password: PasswordSettingResponse;

  @ApiProperty({ description: SiteAdminsDescriptions.radiusSettings })
  @Type(() => RadiusSettingResponse)
  @ValidateNested()
  radius: RadiusSettingResponse;
}

export class RadiusTestResponse {
  @ApiProperty({ description: SiteAdminsDescriptions.radiusTestResponse })
  @IsString()
  message: string;
}
