import { ApiProperty } from '@nestjs/swagger'
import { IsArray, IsBoolean, IsDate, IsString } from 'class-validator'
import { User } from '../../../../shared/types/dtos/auth.dto'
import { UserDescriptions } from '../users.constants'

export class UserResponse {
    @ApiProperty({ description: UserDescriptions.hybridOSId, required: false })
    @IsString()
    id?: User['id']
    @ApiProperty({ description: UserDescriptions.username, required: false })
    @IsString()
    username?: User['username']
    @ApiProperty({ description: UserDescriptions.role, required: false })
    role?: User['role']
    @ApiProperty({
        description: UserDescriptions.password,
        required: false,
    })
    @IsString()
    password?: User['password']
    @ApiProperty({ description: UserDescriptions.old_passwords, required: false })
    @IsArray()
    old_passwords?: User['old_passwords']
    @ApiProperty({
        description: UserDescriptions.pwdUpdatedDate,
        required: false,
    })
    @IsDate()
    pwdUpdatedDate?: User['pwdUpdatedDate']
    @ApiProperty({ description: UserDescriptions.shared_key, required: false })
    @IsString()
    shared_key?: User['shared_key']
    @ApiProperty({ description: UserDescriptions.mfa_enabled, required: false })
    @IsBoolean()
    mfa_enabled?: User['mfa_enabled']
    @ApiProperty({ description: UserDescriptions.version, required: false })
    @IsString()
    version?: User['version']
}
