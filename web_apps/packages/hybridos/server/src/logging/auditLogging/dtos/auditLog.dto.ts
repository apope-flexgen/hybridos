import { IsNotEmpty, IsOptional, IsString } from 'class-validator'

export class AuditLogDTO {
    @IsString()
    @IsNotEmpty()
    modified_field: string

    @IsNotEmpty()
    modified_value: string | boolean

    @IsOptional()
    extraFields?: {
        [key: string]: string | boolean | number
    }
}
