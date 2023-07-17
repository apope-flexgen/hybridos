import { ApiProperty } from '@nestjs/swagger'
import { IsString } from 'class-validator'
import { ErcotOverrideDescriptions } from '../ercotOverride.constants'

export class EditVariableRepnse {
    @ApiProperty({ description: ErcotOverrideDescriptions.returnData })
    @IsString()
    data: string
}
