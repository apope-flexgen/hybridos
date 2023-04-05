import { ApiProperty } from '@nestjs/swagger'
import { AppDescriptions } from '../app.constants'
import { LoginInfo } from '../app.interface'

export class LoginInfoResponse {
    @ApiProperty({description: AppDescriptions.productResponse})
    product: LoginInfo
}
