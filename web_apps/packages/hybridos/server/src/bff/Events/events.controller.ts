import { Controller, Get, Query, UsePipes, ValidationPipe } from '@nestjs/common'
import { ApiTags } from '@nestjs/swagger'
import { EventsService } from './events.service'
import { EventsRequest } from './dtos/events.dto'
import { EventListResponse } from '../../../../shared/types/dtos/events.dto'

@ApiTags('events')
@Controller('events')
export class EventsController {
    constructor(private readonly eventsService: EventsService) {}
    @Get()
    @UsePipes(new ValidationPipe({ transform: true }))
    async events(@Query() query: EventsRequest): Promise<EventListResponse> {
        return await this.eventsService.eventsFiltered(query)
    }
}
