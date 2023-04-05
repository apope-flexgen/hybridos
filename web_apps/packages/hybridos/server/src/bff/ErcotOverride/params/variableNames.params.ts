import { IsNotEmpty, IsString } from 'class-validator'

export class VariableParams {
    @IsNotEmpty()
    @IsString()
    siteId: string
}