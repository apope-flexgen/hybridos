import { MiddlewareConsumer, Module, NestModule, ValidationPipe } from '@nestjs/common'
import { APP_FILTER, APP_GUARD, APP_INTERCEPTOR, APP_PIPE } from '@nestjs/core'
import { MongooseModule } from '@nestjs/mongoose'
import { ScheduleModule } from '@nestjs/schedule'
import { ServeStaticModule } from '@nestjs/serve-static'
import * as path from 'path'
import { AppSettingsModule } from './appSettings/appSettings.module'
import { AuthModule } from './auth/auth.module'
import { AccessTokenAuthGuard } from './auth/guards/accessTokenAuth.guard'
import { RolesGuard } from './auth/guards/roles.guard'
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
        AppEnvModule,
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
    ],
    controllers: [],
    providers: [
        // the order of these guards matters, AccessTokenAuthGuard must execute before RolesGuard
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
            provide: APP_INTERCEPTOR,
            useFactory: (appEnv: AppEnvService) => {
                if (USE_TIMEOUT_INTERCEPTOR) {
                    return new TimeoutInterceptor(appEnv)
                }
                return
            },
            inject: [AppEnvService],
        },
    ],
})
export class AppModule implements NestModule {
    configure(consumer: MiddlewareConsumer) {
        consumer.apply(LoggerMiddleware).forRoutes('*')
    }
}
