import { Module } from '@nestjs/common'
import { AssetsController } from './assets.controller'
import { AssetsService } from './assets.service'
import { ASSETS_SERVICE } from './assets.constants'
import { DBIModule } from 'src/dbi/dbi.module'
import { LayoutsGateway } from 'src/bff/Layouts/layouts.gateway'
import { LayoutsService } from 'src/bff/Layouts/layouts.service'
import { LayoutsModule } from 'src/bff/Layouts/layouts.module'

@Module({
    imports: [DBIModule, LayoutsModule],
    controllers: [AssetsController],
    providers: [
        {
            useClass: AssetsService,
            provide: ASSETS_SERVICE,
        },
        LayoutsGateway,
        LayoutsService,
    ],
})
export class AssetsModule {}
