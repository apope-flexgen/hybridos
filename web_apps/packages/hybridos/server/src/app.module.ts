import {
    MiddlewareConsumer,
    Module,
    NestModule,
    ValidationPipe,
} from '@nestjs/common'
import { APP_FILTER, APP_GUARD, APP_INTERCEPTOR, APP_PIPE } from '@nestjs/core'
import { MongooseModule } from '@nestjs/mongoose'
import { ScheduleModule } from '@nestjs/schedule'
import { ServeStaticModule } from '@nestjs/serve-static'
import * as path from 'path'
import { SiteAdminsModule } from './siteAdmins/siteAdmins.module'
import { AuthModule } from './auth/auth.module'
import { AccessTokenAuthGuard } from './auth/guards/accessTokenAuth.guard'
import { BffModule } from './bff/bff.module'
import { AppEnvModule } from './environment/appEnv.module'
import { AppEnvService } from './environment/appEnv.service'
import { HttpExceptionFilter } from './filters/http-expections.filter'
import { MongoErrorFilter } from './filters/mongoose.filter'
import { FimsModule } from './fims/fims.module'
import { TimeoutInterceptor } from './interceptors/timeout.interceptor'
import { LoggerMiddleware } from './logging/logging.middleware'
import { LoggingModule } from './logging/logging.module'
import { LoggerFilterModule } from './logging/logging_filter/logger-filter.module'
import { PermissionsModule } from './permissions/permissions.module'
import { UsersModule } from './users/users.module'
import { ConfigModule as CustomConfigModule } from './config/config.module'
import { AssetsModule } from './assets/assets.module'
import { DashboardsModule } from './dashboards/dashboards.module'
import { DBIModule } from './dbi/dbi.module'
import { RestModule } from './rest/rest.module'
import { ThrottlerGuard, ThrottlerModule } from '@nestjs/throttler'
import { WebUIConfigModule } from './webuiconfig/webUIConfig.module'
import { LoggingService } from './logging/logging.service'
import { AppExceptionsFilter } from './filters/all-exceptions.filter'
import { ConfigModule } from '@nestjs/config'

@Module({
    imports: [
        ConfigModule.forRoot({
            isGlobal: true,
        }),
        MongooseModule.forRootAsync({
            useFactory: async (appEnv: AppEnvService) => ({
                uri: appEnv.getMongoUri(),
            }),
            inject: [AppEnvService],
        }),
        UsersModule,
        PermissionsModule,
        SiteAdminsModule,
        AuthModule,
        LoggingModule,
        LoggerFilterModule,
        FimsModule,
        AssetsModule,
        DashboardsModule,
        DBIModule,
        AppEnvModule,
        RestModule,
        ScheduleModule.forRoot(),
        BffModule,
        WebUIConfigModule,
        ServeStaticModule.forRootAsync({
            useFactory: async () => {
                if (
                    process.env.NODE_ENV === 'dev' ||
                    process.env.NODE_ENV === 'test'
                ) {
                    return [{}]
                }
                const webUiBuildPAth = process.argv[2]
                return [
                    {
                        rootPath: path.resolve(webUiBuildPAth, 'build'),
                    },
                ]
            },
        }),
        CustomConfigModule,
        ThrottlerModule.forRootAsync({
            inject: [AppEnvService],
            useFactory: async (appEnv: AppEnvService) => ({
                ttl: appEnv.getThrottleTTL(),
                limit: appEnv.getThrottleLimit(),
            }),
        }),
        LoggingModule,
    ],
    controllers: [],
    providers: [
        // the order of these guards matters
        {
            provide: APP_GUARD,
            useClass: AccessTokenAuthGuard,
        },
        {
            provide: APP_PIPE,
            useFactory: () => {
                return new ValidationPipe({ whitelist: true })
            },
        },
        {
            provide: APP_FILTER,
            useClass: AppExceptionsFilter,
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
            provide: APP_GUARD,
            useClass: ThrottlerGuard,
        },
        LoggingService,
    ],
})
export class AppModule implements NestModule {
    configure(consumer: MiddlewareConsumer) {
        consumer.apply(LoggerMiddleware).forRoutes('*')
    }
}
