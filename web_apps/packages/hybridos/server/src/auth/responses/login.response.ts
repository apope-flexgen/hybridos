import { ApiProperty } from '@nestjs/swagger'

import { UserDescriptions } from '../../users/users.constants'
import { AuthDescriptions } from '../auth.constants'

export class ApiLoginResponse {
    @ApiProperty({ description: UserDescriptions.username })
    username: string

    @ApiProperty({ description: UserDescriptions.role, required: false })
    role: string

    @ApiProperty({ description: AuthDescriptions.qrCode, required: false })
    qrCode?: string

    @ApiProperty({ description: AuthDescriptions.mfaRequired, required: false })
    mfaRequired?: boolean

    @ApiProperty({
        description: AuthDescriptions.passwordExpired,
        required: false,
    })
    passwordExpired?: boolean

    @ApiProperty({ description: AuthDescriptions.accessToken, required: false })
    accessToken?: string
}
