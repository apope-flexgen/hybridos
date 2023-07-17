import { ApiProperty } from '@nestjs/swagger'
import { LayoutsDescriptions } from './layouts.constants'

export class LayoutsResponse {
    @ApiProperty({ description: LayoutsDescriptions.layoutResponse })
    layouts: string | Record<string, unknown>
}
