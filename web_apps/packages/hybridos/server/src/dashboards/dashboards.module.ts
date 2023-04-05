import { Module } from '@nestjs/common'
import { DashboardsController } from './dashboards.controller'
import { DashboardsService } from './dashboards.service'
import { DASHBOARDS_SERVICE } from './dashboards.constants'
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
    controllers: [DashboardsController],
    providers: [
        {
            useClass: DashboardsService,
            provide: DASHBOARDS_SERVICE,
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
export class DashboardsModule { }
