import { IsNumber, IsString } from 'class-validator'

export class UpdateConfigDto {
    @IsNumber()
    id: number
    @IsString()
    fileContents: string
}
