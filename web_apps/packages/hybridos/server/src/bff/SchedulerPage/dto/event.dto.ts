import { ApiProperty } from '@nestjs/swagger'
import { Allow, IsArray, IsNumber, IsOptional, IsString } from 'class-validator'
import { SchedulerEvent } from '../../../../../shared/types/dtos/scheduler.dto'
import { SchedulerDescriptions } from '../scheduler.constants'
export class Repeat {
    @ApiProperty({ description: SchedulerDescriptions.event.repeatId })
    @IsOptional()
    @IsString()
    id?: string
    @ApiProperty({ description: SchedulerDescriptions.event.repeatCycle })
    @IsOptional()
    cycle?: 'day' | 'week'
    @ApiProperty({ description: SchedulerDescriptions.event.repeatFrequency })
    @IsOptional()
    @IsNumber()
    frequency?: number
    @ApiProperty({ description: SchedulerDescriptions.event.repeatDayMask })
    @IsOptional()
    day_mask?: number | undefined
    @ApiProperty({ description: SchedulerDescriptions.event.repeatEndTime })
    @IsOptional()
    @IsString()
    end_time?: string
    @ApiProperty({ description: SchedulerDescriptions.event.repeatEndCount })
    @IsOptional()
    @IsNumber()
    end_count?: number
    @ApiProperty({
        description: SchedulerDescriptions.event.repeatExceptions,
        type: String,
        isArray: true,
    })
    @IsOptional()
    @IsArray()
    exceptions?: string[]
}

export class Event {
    @ApiProperty({ description: SchedulerDescriptions.event.id })
    @IsString()
    @IsOptional()
    id?: SchedulerEvent['id']
    @ApiProperty({ description: SchedulerDescriptions.event.duration })
    @IsNumber()
    duration: SchedulerEvent['duration']
    @ApiProperty({ description: SchedulerDescriptions.event.mode })
    @IsString()
    mode: SchedulerEvent['mode']
    @ApiProperty({ description: SchedulerDescriptions.event.startTime })
    @IsString()
    start_time: SchedulerEvent['start_time']
    @ApiProperty({ description: SchedulerDescriptions.event.variables })
    @IsOptional()
    variables?: {
        [key: string]: number | boolean | string
    }
    @ApiProperty({ description: SchedulerDescriptions.event.repeat })
    @IsOptional()
    repeat?: Repeat
}
