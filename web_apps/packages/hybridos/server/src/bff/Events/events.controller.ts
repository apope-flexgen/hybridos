import {
    Controller,
    Get,
    Query,
    UsePipes,
    ValidationPipe,
} from '@nestjs/common'
import {
    ApiDefaultResponse,
    ApiOkResponse,
    ApiSecurity,
    ApiTags,
} from '@nestjs/swagger'
import { EventsService } from './events.service'
import { EventsRequest } from './dtos/events.dto'
import { EventsResponse } from './responses/events.response'
import { DefaultApiError } from 'src/exceptions/defaultResponse.exception'

@ApiTags('events')
@ApiSecurity('bearerAuth')
@ApiDefaultResponse({type: DefaultApiError})
@Controller('events')
export class EventsController {
    constructor(private readonly eventsService: EventsService) {}
    @Get()
    @ApiOkResponse({type: EventsResponse})
    @UsePipes(new ValidationPipe({ transform: true }))
    async events(@Query() query: EventsRequest): Promise<EventsResponse> {
        return await this.eventsService.eventsFiltered(query)
    }
}
