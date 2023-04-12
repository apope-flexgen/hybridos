import { Module } from '@nestjs/common'
import { JwtModule } from '@nestjs/jwt'
import { MongooseModule } from '@nestjs/mongoose'
import { VALID_JWT_SERVICE } from 'src/auth/interfaces/validJWT.service.interface'
import { ValidAccessTokenService } from 'src/auth/validJWT.service'
import { DBI_SERVICE } from 'src/dbi/dbi.constants'
import { DBIModule } from 'src/dbi/dbi.module'
import { DBIService } from 'src/dbi/dbi.service'
import { SiteAdminsController } from './siteAdmins.controller'
import { SiteAdminsService as SiteAdminsService } from './siteAdmins.service'
import { SITE_ADMINS_SERVICE } from './interfaces/siteAdmin.service.interface'
import { SiteAdminsSchema } from './schemas/siteAdmins.schema'

@Module({
    imports: [
        MongooseModule.forFeature([{ name: 'siteadmins', schema: SiteAdminsSchema }]),
        JwtModule.register({
            secret: 'supersecretkey',
        }),
        DBIModule
    ],
    controllers: [SiteAdminsController],
    providers: [
        {
            provide: SITE_ADMINS_SERVICE,
            useClass: SiteAdminsService,
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
    exports: [
        {
            provide: SITE_ADMINS_SERVICE,
            useClass: SiteAdminsService,
        },
    ],
})
export class SiteAdminsModule {}
