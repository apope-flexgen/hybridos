import { Body, Controller, Get, Post } from '@nestjs/common'
import { ConfigFileService } from './configFile.service'
import { ConfigHistoryService } from './configHistory.service'
import { UpdateConfigDto } from './dtos/updateConfig.dto'
import { ConfigHistoryResponse } from './dtos/configHistory.response.dto'

@Controller('config')
export class ConfigFileController {
    constructor(
        private readonly configHistoryService: ConfigHistoryService,
        private readonly configFileService: ConfigFileService
    ) {}
@Get('history')
    async getHistory(): Promise<ConfigHistoryResponse> {
        return await this.configHistoryService.getHistory()
    }
@Get()
    async getAll() {
        return await this.configFileService.getAll()
    }
@Post()
    async updateConfig(@Body() configDto: UpdateConfigDto) {
        return await this.configFileService.updateConfig(configDto.id, configDto.fileContents)
    }
}
