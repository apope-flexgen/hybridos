import { ApiProperty } from '@nestjs/swagger'
import { IsOptional } from 'class-validator'
import { UserDescriptions } from '../users.constants'
import { IsValidRole } from '../validators/IsValidRole'

export class AllUsers {
    @ApiProperty({ description: UserDescriptions.role, required: false })
    @IsValidRole()
    @IsOptional()
    role?: string
}
