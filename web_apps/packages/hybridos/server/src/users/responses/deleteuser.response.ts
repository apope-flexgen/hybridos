import { ApiProperty } from '@nestjs/swagger'
import { IsBoolean } from 'class-validator'
import { UserDescriptions } from '../users.constants'

export class DeleteUserResponse {
    @ApiProperty({ description: UserDescriptions.userDeleted })
    @IsBoolean()
    userDeleted: boolean
}
