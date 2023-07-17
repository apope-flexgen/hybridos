import { Body, Controller, Get, Inject, Post } from '@nestjs/common'
import {
    ApiDefaultResponse,
    ApiOkResponse,
    ApiSecurity,
    ApiTags,
} from '@nestjs/swagger'
import { LayoutsResponse } from './responses'
import { ILayoutsService } from './layouts.interface'
import { LAYOUTS_SERVICE } from './layouts.constants'
import { AddLayout } from './dto/layout.dto'
import { DefaultApiError } from 'src/exceptions/defaultResponse.exception'

@ApiTags('layouts')
@ApiSecurity('bearerAuth')
@ApiDefaultResponse({ type: DefaultApiError })
@Controller('layouts')
export class LayoutsController {
    constructor(
        @Inject(LAYOUTS_SERVICE)
        private readonly layoutsService: ILayoutsService
    ) {}
    @ApiOkResponse({ type: LayoutsResponse })
    @Get()
    async getLayouts() {
        return this.layoutsService.getLayouts()
    }
    @ApiOkResponse({ type: LayoutsResponse })
    @Post()
    async postLayouts(@Body() layout: AddLayout): Promise<LayoutsResponse> {
        return this.layoutsService.postLayouts(layout)
    }
}
