import { ApiProperty } from '@nestjs/swagger'
import { IsMongoId, IsNotEmpty, IsString } from 'class-validator'
import { DeleteUserRequest } from '../../../../shared/types/dtos/auth.dto'
import { UserDescriptions } from '../users.constants'

export class DeleteUserParams {
    @ApiProperty({ description: UserDescriptions.hybridOSId })
    @IsNotEmpty()
    @IsMongoId()
    @IsString()
    id: string
}
