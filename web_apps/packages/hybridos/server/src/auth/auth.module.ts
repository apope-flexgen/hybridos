import { Module } from '@nestjs/common';
import { JwtModule } from '@nestjs/jwt';
import { PassportModule } from '@nestjs/passport';
import { AppEnvModule } from '../environment/appEnv.module';
import { SiteAdminsModule } from '../siteAdmins/siteAdmins.module';
import { UsersModule } from '../users/users.module';
import { AuthController } from './auth.controller';
import { AuthService } from './auth.service';
import { AUTH_SERVICE } from './interfaces/auth.service.interface';
import { MFA_SERVICE } from './interfaces/mfa.service.interface';
import { PASS_EXP_SERVICE } from './interfaces/passExp.service.interface';
import { RADIUS_SERVICE } from './interfaces/radius.service.interface';
import { MfaService } from './mfa.service';
import { PassExpService } from './passExp.service';
import { RadiusService } from './radius.service';
import { AccessTokenMfaStrategy } from './strategies/accessToken.mfa.strategy';
import { AccessTokenPassExpStrategy } from './strategies/accessToken.passExp.strategy';
import { AccessTokenStrategy } from './strategies/accessToken.strategy';
import { LocalStrategy } from './strategies/local.strategy';
import { RadiusStrategy } from './strategies/radius.strategy';
import { TotpStrategy } from './strategies/totp.strategy';
import { RefreshTokenStrategy } from './strategies/refreshToken.strategy';
import { RefreshTokenService } from './refreshToken.service';
import { ValidAccessTokenService } from './validAccessToken.service';
import { BasicAccessStrategy } from './strategies/basicAccessAuth.strategy';
import { AuditLoggingModule } from '../logging/auditLogging/auditLogging.module';
import { APP_ENV_SERVICE, IAppEnvService } from 'src/environment/appEnv.interface';
import { VALID_ACCESS_TOKEN_SERVICE } from 'src/auth/interfaces/validAccessToken.service.interface';

@Module({
  imports: [
    UsersModule,
    PassportModule,
    JwtModule.registerAsync({
      imports: [AppEnvModule],
      useFactory: async (appEnvService: IAppEnvService) => {
        return {
          secret: appEnvService.getJwtSecretKey(),
        };
      },
      inject: [APP_ENV_SERVICE],
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
      provide: VALID_ACCESS_TOKEN_SERVICE,
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
  exports: [VALID_ACCESS_TOKEN_SERVICE],
})
export class AuthModule {}
