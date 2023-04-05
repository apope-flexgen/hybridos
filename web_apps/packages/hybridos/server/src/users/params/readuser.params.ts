import { ApiProperty } from '@nestjs/swagger'
import { IsMongoId, IsNotEmpty } from 'class-validator'
import { GetUserRequest } from '../../../../shared/types/dtos/auth.dto'
import { UserDescriptions } from '../users.constants'

export class ReadUserParams {
    @ApiProperty({ description: UserDescriptions.hybridOSId })
    @IsNotEmpty()
    @IsMongoId()
    id: GetUserRequest['id']
}
