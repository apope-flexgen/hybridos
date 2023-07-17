import { ApiProperty } from '@nestjs/swagger'
import { IsNotEmpty, IsString } from 'class-validator'
import { ErcotOverrideDescriptions } from '../ercotOverride.constants'

export class Site {
    @ApiProperty({ description: ErcotOverrideDescriptions.siteId })
    @IsNotEmpty()
    @IsString()
    id: string
}

export class Variable {
    @ApiProperty({ description: ErcotOverrideDescriptions.variableName })
    @IsNotEmpty()
    @IsString()
    name: string
}