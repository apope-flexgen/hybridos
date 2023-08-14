import {
  Body,
  Controller,
  Get,
  HttpCode,
  Inject,
  Post,
  Req,
  Res,
  UseFilters,
  UseGuards,
} from '@nestjs/common';
import { ApiDefaultResponse, ApiOkResponse, ApiSecurity, ApiTags } from '@nestjs/swagger';
import { Public } from '../decorators/public.decorator';
import { IncorrectCodeFilter } from './filters/incorrectCode.filter';
import { MfaRequiredFilter } from './filters/mfa.filter';
import { PassExpFilter } from './filters/passExp.filter';
import { UnchangedPasswordFilter } from './filters/unchangedPassword.filter';
import { UserNotFoundFilter } from './filters/userNotFound.filter';
import { AuthenticationGuard } from './guards/auth.guard';
import { AccessTokenMfaGuard } from './guards/accessToken.mfa.guard';
import { AccessTokenPassExpGuard } from './guards/accessToken.passExp.guard';
import { RefreshTokenGuard } from './guards/refreshToken.guard';
import { TotpGuard } from './guards/totp.guard';
import { AUTH_SERVICE, IAuthService } from './interfaces/auth.service.interface';
import { ApiLoginResponse } from './responses/login.response';
import { LoginSuccessResponse } from './responses/login.success.response';
import { Logout } from './responses/logout.response';
import { MfaCredentials } from './dto/mfaCredentials.dto';
import { PassExpCredentials } from './dto/passExpCredentials.dto';
import { Request } from 'express';
import { UserFromAccessToken } from 'src/decorators/userFromAccessToken.decorator';
import { User } from 'src/users/dtos/user.dto';
import { Throttle } from '@nestjs/throttler';
import { LOGIN_API_LIMIT, LOGIN_API_TTL } from 'src/environment/appEnv.constants';
import {
  AUDIT_LOGGING_SERVICE,
  IAuditLoggingService,
} from '../logging/auditLogging/interfaces/auditLogging.service.interface';
import { DefaultApiError } from 'src/exceptions/defaultResponse.exception';

@ApiTags('auth')
@ApiDefaultResponse({ type: DefaultApiError })
@Controller('/')
export class AuthController {
  constructor(
    @Inject(AUTH_SERVICE)
    private readonly authService: IAuthService,
    @Inject(AUDIT_LOGGING_SERVICE)
    private readonly auditLoggingService: IAuditLoggingService,
  ) {}
  @ApiOkResponse({ type: ApiLoginResponse })
  @Public()
  @Post('login')
  @HttpCode(200)
  @UseGuards(AuthenticationGuard)
  @Throttle(LOGIN_API_LIMIT, LOGIN_API_TTL)
  @UseFilters(MfaRequiredFilter, PassExpFilter, UserNotFoundFilter)
  async login(@Req() req, @Res({ passthrough: true }) res): Promise<LoginSuccessResponse> {
    const { accessToken, refreshToken, ...user } = await this.authService.login(req.user);
    const data = {
      modified_field: 'user login',
      modified_value: true,
    };
    await this.auditLoggingService.postAuditLog(data, user);

    res.cookie('refreshToken', refreshToken, {
      sameSite: true,
      httpOnly: true,
      secure: true,
    });
    return {
      accessToken: `Bearer ${accessToken}`,
      ...user,
    };
  }
  @ApiOkResponse({ type: LoginSuccessResponse })
  @Public()
  @Post('login/mfa')
  @HttpCode(200)
  @UseGuards(AccessTokenMfaGuard, TotpGuard)
  @Throttle(LOGIN_API_LIMIT, LOGIN_API_TTL)
  @UseFilters(IncorrectCodeFilter)
  async mfa(
    @Res({ passthrough: true }) res,
    @Body() mfaCredentials: MfaCredentials,
  ): Promise<LoginSuccessResponse> {
    const { accessToken, refreshToken, ...user } = await this.authService.loginMFA(
      mfaCredentials.username,
    );
    const data = {
      modified_field: 'user login mfa',
      modified_value: true,
    };
    await this.auditLoggingService.postAuditLog(data, user);

    res.cookie('refreshToken', refreshToken, {
      sameSite: true,
      httpOnly: true,
      secure: true,
    });
    return {
      accessToken: `Bearer ${accessToken}`,
      ...user,
    };
  }
  @ApiOkResponse({ type: LoginSuccessResponse })
  @Public()
  @Post('login/passExp')
  @HttpCode(200)
  @UseGuards(AccessTokenPassExpGuard)
  @Throttle(LOGIN_API_LIMIT, LOGIN_API_TTL)
  @UseFilters(MfaRequiredFilter, UnchangedPasswordFilter)
  async passExpLogin(
    @Res({ passthrough: true }) res,
    @Body() passExpCredentials: PassExpCredentials,
  ): Promise<LoginSuccessResponse> {
    const { accessToken, refreshToken, ...user } = await this.authService.loginPassExp(
      passExpCredentials,
    );
    const data = {
      modified_field: 'user login pass expired',
      modified_value: true,
    };
    await this.auditLoggingService.postAuditLog(data, user);

    res.cookie('refreshToken', refreshToken, {
      sameSite: true,
      httpOnly: true,
      secure: true,
    });
    return {
      accessToken: `Bearer ${accessToken}`,
      ...user,
    };
  }

  @ApiOkResponse({ type: LoginSuccessResponse })
  @Get('refresh_token')
  @Public()
  @UseGuards(RefreshTokenGuard)
  async refreshTokens(@Req() req, @Res({ passthrough: true }) res): Promise<LoginSuccessResponse> {
    const { accessToken, refreshToken } = await this.authService.refreshTokens(
      req.user.sub,
      req.user.role,
      req.user.refreshToken,
    );
    const data = {
      modified_field: 'user login refresh token',
      modified_value: true,
    };

    await this.auditLoggingService.postAuditLog(data, {
      username: req.user.username,
      role: req.user.role,
    });

    res.cookie('refreshToken', refreshToken, {
      sameSite: true,
      httpOnly: true,
      secure: true,
    });
    return {
      accessToken: accessToken,
      username: req.user.sub,
      role: req.user.role,
    };
  }
  @ApiOkResponse({ type: LoginSuccessResponse })
  @ApiSecurity('bearerAuth')
  @Get('authenticate-user-token')
  async authenticateUserToken(@Req() req): Promise<LoginSuccessResponse> {
    const response = this.authService.authUserToken(req.user);
    const data = {
      modified_field: 'authentication token login',
      modified_value: true,
    };
    await this.auditLoggingService.postAuditLog(data, {
      username: req.user.username,
      role: req.user.role,
    });

    return response;
  }
  @ApiOkResponse({ type: Logout })
  @ApiSecurity('bearerAuth')
  @Post('logout')
  @HttpCode(200)
  async logout(@Req() request: Request, @UserFromAccessToken() user: User): Promise<Logout> {
    const response = this.authService.logout(request);
    const data = {
      modified_field: 'user logout',
      modified_value: true,
    };
    await this.auditLoggingService.postAuditLog(data, user);

    return response;
  }
}
