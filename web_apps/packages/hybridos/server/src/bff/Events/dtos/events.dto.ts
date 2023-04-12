import { IsNumberString, IsOptional } from 'class-validator'
import { EventsRequestParams } from '../../../../../shared/types/dtos/events.dto'

export class EventsRequest {
    @IsOptional()
    startTime: EventsRequestParams['startTime']
    @IsOptional()
    endTime: EventsRequestParams['endTime']
    @IsOptional()
    source: EventsRequestParams['source']
    @IsOptional()
    severity: EventsRequestParams['severity']
    @IsOptional()
    search: EventsRequestParams['search']
    @IsOptional()
    @IsNumberString()
    limit: EventsRequestParams['limit']
    @IsOptional()
    @IsNumberString()
    page: EventsRequestParams['page']
    @IsOptional()
    @IsNumberString()
    order: EventsRequestParams['order']
    @IsOptional()
    orderBy: EventsRequestParams['orderBy']
}
