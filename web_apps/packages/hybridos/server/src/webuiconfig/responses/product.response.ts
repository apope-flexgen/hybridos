import { ApiProperty } from '@nestjs/swagger'
import { IsOptional } from 'class-validator'
import { WebUIConfigDescriptions } from '../webUIConfig.constants'
import { LoginInfo } from '../webUIConfig.interface'

export class LoginInfoResponse {
    @ApiProperty({description: WebUIConfigDescriptions.productResponse})
    product: LoginInfo['product']
    @ApiProperty({description: WebUIConfigDescriptions.customer})
    @IsOptional()
    customer: LoginInfo['customer']
    @ApiProperty({description: WebUIConfigDescriptions.siteName})
    @IsOptional()
    siteName: LoginInfo['siteName']
    @ApiProperty({description: WebUIConfigDescriptions.hardware})
    @IsOptional()
    server: LoginInfo['server']
}
