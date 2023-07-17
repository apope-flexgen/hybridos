import { ApiProperty } from '@nestjs/swagger'
import { IsNotEmpty, IsString } from 'class-validator'
import { SchedulerDescriptions } from '../scheduler.constants'

export class SiteId {
    @ApiProperty({ description: SchedulerDescriptions.event.siteId })
    @IsNotEmpty()
    @IsString()
    siteId: string
}

export class EventId {
    @ApiProperty({ description: SchedulerDescriptions.event.eventId })
    @IsNotEmpty()
    @IsString()
    eventId: string
}