import { ApiProperty } from "@nestjs/swagger"

export class RestSetValueResponse {
    @ApiProperty({ description: 'HTTP status code', required: false })
    status?: number
    @ApiProperty({ description: 'Status desciption', required: false })
    statusString?: string
    @ApiProperty({ description: 'HTTP method', required: false })
    method?: string
    @ApiProperty({ description: 'Target URI', required: false })
    uri?: string
    @ApiProperty({ description: 'Value set', required: false })
    value?: any
}
