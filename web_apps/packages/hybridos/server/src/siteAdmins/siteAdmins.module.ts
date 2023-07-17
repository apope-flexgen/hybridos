import { Module } from '@nestjs/common'
import { MongooseModule } from '@nestjs/mongoose'
import { SiteAdminsController } from './siteAdmins.controller'
import { SiteAdminsService as SiteAdminsService } from './siteAdmins.service'
import { SITE_ADMINS_SERVICE } from './interfaces/siteAdmin.service.interface'
import { SiteAdminsSchema } from './schemas/siteAdmins.schema'
import { AuditLoggingModule } from '../logging/auditLogging/auditLogging.module'

@Module({
    imports: [
        MongooseModule.forFeature([
            { name: 'siteadmins', schema: SiteAdminsSchema },
        ]),
        AuditLoggingModule,
    ],
    controllers: [SiteAdminsController],
    providers: [
        {
            provide: SITE_ADMINS_SERVICE,
            useClass: SiteAdminsService,
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
