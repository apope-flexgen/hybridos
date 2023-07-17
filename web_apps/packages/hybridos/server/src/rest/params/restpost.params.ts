import { ApiProperty } from '@nestjs/swagger'
import { IsNotEmpty } from 'class-validator'

export class RestPostParams {
    @ApiProperty({ description: 'URI' })
    @IsNotEmpty()
    endpoint: string
}
