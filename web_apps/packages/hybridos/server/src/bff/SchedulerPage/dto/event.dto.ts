import { Allow, IsNumber, IsOptional, IsString } from 'class-validator'
import { SchedulerEvent } from '../../../../../shared/types/dtos/scheduler.dto'
export class Event {
    @IsString()
    id: SchedulerEvent['id']
    @IsNumber()
    duration: SchedulerEvent['duration']
    @IsString()
    mode: SchedulerEvent['mode']
    @IsString()
    start_time: SchedulerEvent['start_time']
    @Allow()
    variables?: SchedulerEvent['variables']
    @IsOptional()
    repeat?: SchedulerEvent['repeat']
}
