import { ApiProperty } from '@nestjs/swagger'
import { UserDescriptions } from '../users.constants'

export class DeleteUserResponse {
    @ApiProperty({ description: UserDescriptions.userDeleted })
    userDeleted: boolean
}
