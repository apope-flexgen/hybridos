import { ApiProperty } from '@nestjs/swagger'
import { UserDescriptions } from '../users.constants'
import { UserResponse } from './user.response'

export class AllUsersResponse {
    @ApiProperty({
        description: UserDescriptions.allUsersResponse,
        isArray: true,
        type: UserResponse,
    })
    users: Array<UserResponse>
}
