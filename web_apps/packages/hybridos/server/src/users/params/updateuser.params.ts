import { ApiProperty } from '@nestjs/swagger'
import { IsMongoId, IsNotEmpty } from 'class-validator'
import { UpdateUserRequest } from '../../../../shared/types/dtos/auth.dto'
import { UserDescriptions } from '../users.constants'

export class UpdateUserParams {
    @ApiProperty({ description: UserDescriptions.hybridOSId })
    @IsNotEmpty()
    @IsMongoId()
    id: UpdateUserRequest['id']
}
