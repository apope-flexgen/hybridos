import { ApiProperty } from '@nestjs/swagger'
import { IsNotEmpty } from 'class-validator'

export class RestDeleteParams {
    @ApiProperty({ description: 'URI' })
    @IsNotEmpty()
    endpoint: string
}
