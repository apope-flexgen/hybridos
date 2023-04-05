import { Module } from '@nestjs/common'
import { JwtModule } from '@nestjs/jwt'
import { VALID_JWT_SERVICE } from 'src/auth/interfaces/validJWT.service.interface'
import { ValidAccessTokenService } from 'src/auth/validJWT.service'
import { DBI_SERVICE } from 'src/dbi/dbi.constants'
import { DBIModule } from 'src/dbi/dbi.module'
import { DBIService } from 'src/dbi/dbi.service'
import { FimsModule } from '../../../fims/fims.module'
import { AssetsPageGateway } from './assetsPage.gateway'
import { AssetsPageService } from './assetsPage.service'

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
        AssetsPageService,
        AssetsPageGateway,
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
export class AssetsPageModule {}
