import { Module } from '@nestjs/common'
import { DashboardGateway } from './dashboard.gateway'
import { DashboardService } from './dashboard.service'
import { FimsModule } from '../../../fims/fims.module'
import { DBIModule } from 'src/dbi/dbi.module'
import { DBI_SERVICE } from 'src/dbi/dbi.constants'
import { DBIService } from 'src/dbi/dbi.service'
import { JwtModule } from '@nestjs/jwt'
import { VALID_JWT_SERVICE } from 'src/auth/interfaces/validJWT.service.interface'
import { ValidAccessTokenService } from 'src/auth/validJWT.service'

@Module({
    imports: [
        FimsModule,
        DBIModule,
        JwtModule.register({
            secret: 'supersecretkey',
        }),
    ],
    controllers: [],
    providers: [
        DashboardService,
        DashboardGateway,
        {
            provide: DBI_SERVICE,
            useClass: DBIService,
        },
        {
            provide: VALID_JWT_SERVICE,
            useClass: ValidAccessTokenService,
        },
    ],
})
export class DashboardModule {}
