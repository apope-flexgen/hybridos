import { IsNotEmpty, IsString } from 'class-validator'

export class EditOverrideParams {
    @IsNotEmpty()
    @IsString()
    siteId: string
    @IsNotEmpty()
    @IsString()
    variableName: string
}