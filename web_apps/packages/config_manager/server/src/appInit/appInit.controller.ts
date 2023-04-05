import { Controller, Get } from '@nestjs/common'
import { AppInitService } from './appInit.service'

@Controller()
export class AppInitController {
    constructor(private readonly appInitService: AppInitService) {}
    @Get('app')
    getApp() {
        return this.appInitService.getSystemConfiguration()
    }
}
