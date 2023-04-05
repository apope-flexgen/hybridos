import { Module } from '@nestjs/common'
import { AssetsController } from './assets.controller'
import { AssetsService } from './assets.service'
import { ASSETS_SERVICE } from './assets.constants'
import { LayoutsService } from 'src/layouts/layouts.service'
import { LAYOUTS_SERVICE } from 'src/layouts/layouts.constants'
import { JwtModule } from '@nestjs/jwt'
import { VALID_JWT_SERVICE } from 'src/auth/interfaces/validJWT.service.interface'
import { ValidAccessTokenService } from 'src/auth/validJWT.service'
import { DBIModule } from 'src/dbi/dbi.module'
import { DBI_SERVICE } from 'src/dbi/dbi.constants'
import { DBIService } from 'src/dbi/dbi.service'

@Module({
    imports: [
        JwtModule.register({
            secret: 'supersecretkey',
        }),
        DBIModule
    ],
    controllers: [AssetsController],
    providers: [
        {
            useClass: AssetsService,
            provide: ASSETS_SERVICE,
        },
        {
            useClass: LayoutsService,
            provide: LAYOUTS_SERVICE,
        },
        {
            provide: VALID_JWT_SERVICE,
            useClass: ValidAccessTokenService,
        },
        {
            provide: DBI_SERVICE,
            useClass: DBIService,
        },
    ],
    exports: [],
})
export class AssetsModule { }
