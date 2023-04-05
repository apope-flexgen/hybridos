import { ApiProperty } from '@nestjs/swagger'
import { User } from '../../../../shared/types/dtos/auth.dto'
import { UserDescriptions } from '../users.constants'

export class UserResponse {
    @ApiProperty({ description: UserDescriptions.hybridOSId, required: false })
    id?: User['id']
    @ApiProperty({ description: UserDescriptions.username, required: false })
    username?: User['username']
    @ApiProperty({ description: UserDescriptions.role, required: false })
    role?: User['role']
    @ApiProperty({
        description: UserDescriptions.password,
        required: false,
    })
    password?: User['password']
    @ApiProperty({ description: UserDescriptions.old_passwords, required: false })
    old_passwords?: User['old_passwords']
    @ApiProperty({
        description: UserDescriptions.pwdUpdatedDate,
        required: false,
    })
    pwdUpdatedDate?: User['pwdUpdatedDate']
    @ApiProperty({ description: UserDescriptions.shared_key, required: false })
    shared_key?: User['shared_key']
    @ApiProperty({ description: UserDescriptions.mfa_enabled, required: false })
    mfa_enabled?: User['mfa_enabled']
    @ApiProperty({ description: UserDescriptions.version, required: false })
    version?: User['version']
}
