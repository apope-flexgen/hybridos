import { Module } from '@nestjs/common'
import { JwtModule } from '@nestjs/jwt'
import { PassportModule } from '@nestjs/passport'
import { AppEnvModule } from '../environment/appEnv.module'
import { SiteAdminsModule } from '../siteAdmins/siteAdmins.module'
import { UsersModule } from '../users/users.module'
import { AuthController } from './auth.controller'
import { AuthService } from './auth.service'
import { AUTH_SERVICE } from './interfaces/auth.service.interface'
import { MFA_SERVICE } from './interfaces/mfa.service.interface'
import { PASS_EXP_SERVICE } from './interfaces/passExp.service.interface'
import { RADIUS_SERVICE } from './interfaces/radius.service.interface'
import { VALID_JWT_SERVICE } from './interfaces/validJWT.service.interface'
import { MfaService } from './mfa.service'
import { PassExpService } from './passExp.service'
import { RadiusService } from './radius.service'
import { AccessTokenMfaStrategy } from './strategies/accessToken.mfa.strategy'
import { AccessTokenPassExpStrategy } from './strategies/accessToken.passExp.strategy'
import { AccessTokenStrategy } from './strategies/accessToken.strategy'
import { LocalStrategy } from './strategies/local.strategy'
import { RadiusStrategy } from './strategies/radius.strategy'
import { TotpStrategy } from './strategies/totp.strategy'
import { RefreshTokenStrategy } from './strategies/refreshToken.strategy'
import { RefreshTokenService } from './refreshTokenService'
import { ValidAccessTokenService } from './validJWT.service'
import { BasicAccessStrategy } from './strategies/basicAccessAuth.strategy'
import { AppEnvService } from 'src/environment/appEnv.service'
import { AuditLoggingModule } from '../logging/auditLogging/auditLogging.module'

@Module({
    imports: [
        UsersModule,
        PassportModule,
        JwtModule.registerAsync({
            imports: [AppEnvModule],
            useFactory: async (appEnvService: AppEnvService) => {
                return {
                    secret: appEnvService.getJwtSecretKey(),
                }
            },
            inject: [AppEnvService],
        }),
        SiteAdminsModule,
        AppEnvModule,
        AuditLoggingModule,
    ],
    controllers: [AuthController],
    providers: [
        {
            provide: AUTH_SERVICE,
            useClass: AuthService,
        },
        {
            provide: MFA_SERVICE,
            useClass: MfaService,
        },
        {
            provide: PASS_EXP_SERVICE,
            useClass: PassExpService,
        },
        {
            provide: RADIUS_SERVICE,
            useClass: RadiusService,
        },
        {
            provide: VALID_JWT_SERVICE,
            useClass: ValidAccessTokenService,
        },
        RefreshTokenService,
        LocalStrategy,
        AccessTokenStrategy,
        RefreshTokenStrategy,
        AccessTokenMfaStrategy,
        AccessTokenPassExpStrategy,
        TotpStrategy,
        RadiusStrategy,
        BasicAccessStrategy,
    ],
    exports: [
        {
            provide: VALID_JWT_SERVICE,
            useClass: ValidAccessTokenService,
        },
    ],
})
export class AuthModule {}
