import { MiddlewareConsumer, Module, NestModule, ValidationPipe } from '@nestjs/common'
import { APP_FILTER, APP_GUARD, APP_INTERCEPTOR, APP_PIPE } from '@nestjs/core'
import { MongooseModule } from '@nestjs/mongoose'
import { ScheduleModule } from '@nestjs/schedule'
import { ServeStaticModule } from '@nestjs/serve-static'
import * as path from 'path'
import { AppService } from './app.service'
import { APP_SERVICE } from './app.service.interface'
import { AppSettingsModule } from '../appSettings/appSettings.module'
import { AuthModule } from '../auth/auth.module'
import { AccessTokenAuthGuard } from '../auth/guards/accessTokenAuth.guard'
import { RolesGuard } from '../auth/guards/roles.guard'
import { BffModule } from '../bff/bff.module'
import { AppEnvModule } from '../environment/appEnv.module'
import { AppEnvService } from '../environment/appEnv.service'
import { HttpExceptionFilter } from '../filters/http-expections.filter'
import { MongoErrorFilter } from '../filters/mongoose.filter'
import { FimsModule } from '../fims/fims.module'
import { TimeoutInterceptor } from '../interceptors/timeout.interceptor'
import { LoggerMiddleware } from '../logging/logging.middleware'
import { LoggingModule } from '../logging/logging.module'
import { LoggerFilterModule } from '../logging/logging_filter/logger-filter.module'
import { PermissionsModule } from '../permissions/permissions.module'
import { AppController } from './app.controller'
import { UsersModule } from '../users/users.module'
import { RestModule } from '../rest/rest.module'
import { LayoutsModule } from '../layouts/layouts.module'
import { AssetsModule } from '../assets/assets.module'
import { DashboardsModule } from 'src/dashboards/dashboards.module'
import { VALID_JWT_SERVICE } from 'src/auth/interfaces/validJWT.service.interface'
import { JwtModule } from '@nestjs/jwt'
import { ValidAccessTokenService } from 'src/auth/validJWT.service'
import { ThrottlerGuard, ThrottlerModule } from '@nestjs/throttler'
import { DBIModule } from 'src/dbi/dbi.module'

@Module({
    imports: [
        MongooseModule.forRootAsync({
            useFactory: async (appEnv: AppEnvService) => ({
                uri: appEnv.getMongoUri(),
            }),
            inject: [AppEnvService],
        }),
        UsersModule,
        PermissionsModule,
        AppSettingsModule,
        AuthModule,
        LoggingModule,
        LoggerFilterModule,
        FimsModule,
        LayoutsModule,
        AssetsModule,
        DashboardsModule,
        DBIModule,
        AppEnvModule,
        RestModule,
        ScheduleModule.forRoot(),
        BffModule,
        ServeStaticModule.forRootAsync({
            useFactory: async () => {
                const webUiBuildPAth = process.argv[2]
                if (webUiBuildPAth) {
                    return [
                        {
                            rootPath: path.resolve(webUiBuildPAth, 'build'),
                        },
                    ]
                }
                return [{}]
            },
        }),
        JwtModule.register({
            secret: 'supersecretkey',
        }),
        ThrottlerModule.forRootAsync({
            inject: [AppEnvService],
            useFactory: async (appEnv: AppEnvService) => ({
                ttl: appEnv.getThrottleTTL(),
                limit: appEnv.getThrottleLimit(),
            }),
        })
    ],
    controllers: [AppController],
    providers: [
        // the order of these guards matters, JwtAuthGuard must execute before RolesGuard
        {
            provide: APP_GUARD,
            useClass: AccessTokenAuthGuard,
        },
        {
            provide: APP_GUARD,
            useClass: RolesGuard,
        },
        {
            provide: APP_PIPE,
            useFactory: () => {
                return new ValidationPipe({ whitelist: true })
            },
        },
        {
            provide: APP_FILTER,
            useClass: HttpExceptionFilter,
        },
        {
            provide: APP_FILTER,
            useClass: MongoErrorFilter,
        },
        {
            provide: APP_SERVICE,
            useClass: AppService,
        },
        {
            provide: APP_INTERCEPTOR,
            useFactory: (appEnv: AppEnvService) => {
                if (USE_TIMEOUT_INTERCEPTOR) {
                    return new TimeoutInterceptor(appEnv)
                }
                return
            },
            inject: [AppEnvService],
        },
        {
            provide: VALID_JWT_SERVICE,
            useClass: ValidAccessTokenService,
        },
        {
            provide: APP_GUARD,
            useClass: ThrottlerGuard
        }
    ],
})
export class AppModule implements NestModule {
    configure(consumer: MiddlewareConsumer) {
        consumer.apply(LoggerMiddleware).forRoutes('*')
    }
}
