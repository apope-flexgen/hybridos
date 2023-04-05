import { Body, Controller, Get, Inject, Post, Req } from '@nestjs/common'
import { ApiOkResponse, ApiTags } from '@nestjs/swagger';
import { LayoutsResponse } from './responses';
import { ILayoutsService } from './layouts.interface';
import { LAYOUTS_SERVICE } from './layouts.constants';
import { UserFromAccessToken } from 'src/decorators/userFromAccessToken.decorator';
import { User } from 'src/users/dtos/user.dto';

@ApiTags('layouts')
@Controller('layouts')
export class LayoutsController {
    constructor(
        @Inject(LAYOUTS_SERVICE)
        private readonly layoutsService: ILayoutsService
    ) { }
    @ApiOkResponse({ type: LayoutsResponse })
    @Get()
    async getLayouts() {
        return this.layoutsService.getLayouts()
    }
    @ApiOkResponse({ type: LayoutsResponse })
    @Post()
    async postLayouts(@Body() data, @UserFromAccessToken() user: User): Promise<LayoutsResponse> {
        return this.layoutsService.postLayouts(data, user)
    }
}
