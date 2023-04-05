import { Module } from '@nestjs/common'
import { AppInitController } from './appInit.controller'
import { AppInitService } from './appInit.service'

@Module({
    imports: [],
    providers: [AppInitService],
    controllers: [AppInitController],
})
export class AppInitModule {}
