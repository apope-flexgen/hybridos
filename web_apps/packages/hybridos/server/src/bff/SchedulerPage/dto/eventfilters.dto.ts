import { IsString } from 'class-validator'
import { EventsRequestParams } from '../../../../../shared/types/dtos/events.dto'
export class EventFilters {
    @IsString()
    startTime: EventsRequestParams['startTime']
    @IsString()
    endTime: EventsRequestParams['endTime']
}
