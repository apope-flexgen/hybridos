import { ApiProperty } from '@nestjs/swagger'
import { IsArray, IsNotEmpty, IsNumber, IsString } from 'class-validator'
import { EventsDescriptions } from '../events.constants'

export class Event {
    @ApiProperty({ description: EventsDescriptions.eventId })
    @IsString()
    id: string
    @ApiProperty({ description: EventsDescriptions.eventSeverity })
    @IsNotEmpty()
    severity: any
    @ApiProperty({ description: EventsDescriptions.eventSource })
    @IsString()
    source: string
    @ApiProperty({ description: EventsDescriptions.eventMessage })
    @IsString()
    message: string
    @ApiProperty({ description: EventsDescriptions.timestamp })
    @IsString()
    timestamp: string
}

export class EventsResponse {
    @ApiProperty({ description: EventsDescriptions.count })
    @IsNumber()
    count: number
    @ApiProperty({ description: EventsDescriptions.data, type: Event, isArray: true })
    @IsArray()
    data: Event []
}
