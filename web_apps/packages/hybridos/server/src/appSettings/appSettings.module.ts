import { Module } from '@nestjs/common'
import { JwtModule } from '@nestjs/jwt'
import { MongooseModule } from '@nestjs/mongoose'
import { VALID_JWT_SERVICE } from 'src/auth/interfaces/validJWT.service.interface'
import { ValidAccessTokenService } from 'src/auth/validJWT.service'
import { DBI_SERVICE } from 'src/dbi/dbi.constants'
import { DBIModule } from 'src/dbi/dbi.module'
import { DBIService } from 'src/dbi/dbi.service'
import { AppSettingsController } from './appSettings.controller'
import { AppSettingsService } from './appSettings.service'
import { APP_SETTINGS_SERVICE } from './interfaces/appSetting.service.interface'
import { AppSettingSchema } from './schemas/appSetting.schema'

@Module({
    imports: [
        MongooseModule.forFeature([{ name: 'appsettings', schema: AppSettingSchema }]),
        JwtModule.register({
            secret: 'supersecretkey',
        }),
        DBIModule
    ],
    controllers: [AppSettingsController],
    providers: [
        {
            provide: APP_SETTINGS_SERVICE,
            useClass: AppSettingsService,
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
            provide: APP_SETTINGS_SERVICE,
            useClass: AppSettingsService,
        },
    ],
})
export class AppSettingsModule {}
