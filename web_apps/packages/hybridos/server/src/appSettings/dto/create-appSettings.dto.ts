import { ApiProperty } from '@nestjs/swagger'
import { Type } from 'class-transformer'
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
} from 'class-validator'

import { AppSettingsDescriptions } from '../appSettings.constants'
import { ValidPassExpInterval } from '../validators/passwordExpirationInterval.validator'

class PasswordSettingDto {
    @ApiProperty({ description: AppSettingsDescriptions.mfaIsEnabled })
    @IsBoolean()
    multi_factor_authentication: boolean

    @ApiProperty({ description: AppSettingsDescriptions.passwordExpiration })
    @IsBoolean()
    password_expiration: boolean

    @ApiProperty({ description: AppSettingsDescriptions.passwordMinLength })
    @IsInt()
    @Min(8)
    @Max(96)
    minimum_password_length: number

    @ApiProperty({ description: AppSettingsDescriptions.passwordMaxLength })
    @IsInt()
    @Min(16)
    @Max(1028)
    maximum_password_length: number

    @ApiProperty({ description: AppSettingsDescriptions.passExpInt })
    @ValidPassExpInterval()
    password_expiration_interval: string

    @ApiProperty({ description: AppSettingsDescriptions.passOldPasswords })
    @IsInt()
    @Min(0)
    @Max(50)
    old_passwords: number

    @ApiProperty({ description: AppSettingsDescriptions.passRegEx })
    @IsString()
    password_regular_expression: string

    @ApiProperty({ description: AppSettingsDescriptions.lowercase })
    @IsBoolean()
    lowercase: boolean

    @ApiProperty({ description: AppSettingsDescriptions.uppercase })
    @IsBoolean()
    uppercase: boolean

    @ApiProperty({ description: AppSettingsDescriptions.digit })
    @IsBoolean()
    digit: boolean

    @ApiProperty({ description: AppSettingsDescriptions.special })
    @IsBoolean()
    special: boolean
}
class RadiusSettingDto {
    @ApiProperty({ description: AppSettingsDescriptions.radiusIsEnabled })
    @IsBoolean()
    is_enabled: boolean

    @ApiProperty({ description: AppSettingsDescriptions.radiusIp })
    @IsIP()
    ip_address: string

    @ApiProperty({ description: AppSettingsDescriptions.radiusPort })
    @IsPort()
    port: string

    @ApiProperty({ description: AppSettingsDescriptions.radiusSecretPhrase })
    @IsAscii()
    @IsNotEmpty()
    secret_phrase: string

    @ApiProperty({ description: AppSettingsDescriptions.radiusWaitTime })
    @IsInt()
    @IsPositive()
    wait_time: number

    @ApiProperty({ description: AppSettingsDescriptions.radiusLocalAuthDisabled })
    @IsBoolean()
    is_local_auth_disabled: boolean
}

export class CreateAppSettingsDto {
    @ApiProperty({ description: AppSettingsDescriptions.passwordSettings })
    @Type(() => PasswordSettingDto)
    @ValidateNested()
    password: PasswordSettingDto

    @ApiProperty({ description: AppSettingsDescriptions.radiusSettings })
    @Type(() => RadiusSettingDto)
    @ValidateNested()
    radius: RadiusSettingDto
}
