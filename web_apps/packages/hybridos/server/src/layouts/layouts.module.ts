import { Module } from '@nestjs/common'
import { LayoutsController } from './layouts.controller'
import { LayoutsService } from './layouts.service'
import { LAYOUTS_SERVICE } from './layouts.constants'
import { VALID_JWT_SERVICE } from 'src/auth/interfaces/validJWT.service.interface'
import { ValidAccessTokenService } from 'src/auth/validJWT.service'
import { JwtModule } from '@nestjs/jwt'
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
    controllers: [LayoutsController],
    providers: [
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
export class LayoutsModule { }
