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
} from '@nestjs/common'
import { ApiOkResponse, ApiTags } from '@nestjs/swagger'
import { Public } from '../decorators/public.decorator'
import { IncorrectCodeFilter } from './filters/incorrectCode.filter'
import { MfaRequiredFilter } from './filters/mfa.filter'
import { PassExpFilter } from './filters/passExp.filter'
import { UnchangedPasswordFilter } from './filters/unchangedPassword.filter'
import { UserNotFoundFilter } from './filters/userNotFound.filter'
import { AuthenticationGuard } from './guards/auth.guard'
import { AccessTokenMfaGuard } from './guards/accessToken.mfa.guard'
import { AccessTokenPassExpGuard } from './guards/accessToken.passExp.guard'
import { RefreshTokenGuard } from './guards/refreshToken.guard'
import { TotpGuard } from './guards/totp.guard'
import { AUTH_SERVICE, IAuthService } from './interfaces/auth.service.interface'
import { ApiLoginPassExpResponse } from './responses/login.passExp.response'
import { ApiLoginResponse } from './responses/login.response'
import { LoginSuccessResponse } from './responses/login.success.response'
import { Logout } from './responses/logout.response'
import { MfaCredentials } from './dto/mfaCredentials.dto'
import { PassExpCredentials } from './dto/passExpCredentials.dto'
import { Request } from 'express'
import { DBI_SERVICE } from 'src/dbi/dbi.constants'
import { IDBIService } from 'src/dbi/dbi.interface'
import { UserFromAccessToken } from 'src/decorators/userFromAccessToken.decorator'
import { User } from 'src/users/dtos/user.dto'

@ApiTags('auth')
@Controller('/')
export class AuthController {
    constructor(
        @Inject(AUTH_SERVICE)
        private readonly authService: IAuthService,
        @Inject(DBI_SERVICE)
        private readonly dbiService: IDBIService,
    ) {}
    @ApiOkResponse({ type: ApiLoginResponse })
    @Public()
    @Post('login')
    @HttpCode(200)
    @UseGuards(AuthenticationGuard)
    @UseFilters(MfaRequiredFilter, PassExpFilter, UserNotFoundFilter)
    async login(
        @Req() req,
        @Res({ passthrough: true }) res,
    ): Promise<LoginSuccessResponse> {
        const { accessToken, refreshToken, ...user } = await this.authService.login(req.user);
        const data = { 
            username: user.username,
            userrole: user.role,
            modified_field: 'user login',
            modified_value: true
        }
        await this.dbiService.postUIConfigAuditLog(data);
        res.cookie('refreshToken', refreshToken, {
            sameSite: true,
            httpOnly: true,
            secure: true,
        })
        return {
            accessToken: `Bearer ${accessToken}`,
            ...user,
        }
    }
    @ApiOkResponse({ type: LoginSuccessResponse })
    @Public()
    @Post('login/mfa')
    @HttpCode(200)
    @UseGuards(AccessTokenMfaGuard, TotpGuard)
    @UseFilters(IncorrectCodeFilter)
    async mfa(
        @Res({ passthrough: true }) res,
        @Body() mfaCredentials: MfaCredentials
    ): Promise<LoginSuccessResponse> {
        const { accessToken, refreshToken, ...user } = await this.authService.loginMFA(
            mfaCredentials.username
        )
        const data = { 
            username: user.username,
            userrole: user.role,
            modified_field: 'user login mfa',
            modified_value: true
        }
        await this.dbiService.postUIConfigAuditLog(data);
        res.cookie('refreshToken', refreshToken, {
            sameSite: true,
            httpOnly: true,
            secure: true,
        })
        return {
            accessToken: `Bearer ${accessToken}`,
            ...user,
        }
    }
    @ApiOkResponse({ type: ApiLoginPassExpResponse })
    @Public()
    @Post('login/passExp')
    @HttpCode(200)
    @UseGuards(AccessTokenPassExpGuard)
    @UseFilters(MfaRequiredFilter, UnchangedPasswordFilter)
    async passExpLogin(
        @Res({ passthrough: true }) res,
        @Body() passExpCredentials: PassExpCredentials
    ): Promise<LoginSuccessResponse> {
        const { accessToken, refreshToken, ...user } = await this.authService.loginPassExp(
            passExpCredentials
        )
        const data = { 
            username: user.username,
            userrole: user.role,
            modified_field: 'user login pass expired',
            modified_value: true
        }
        await this.dbiService.postUIConfigAuditLog(data);
        res.cookie('refreshToken', refreshToken, {
            sameSite: true,
            httpOnly: true,
            secure: true,
        })
        return {
            accessToken: `Bearer ${accessToken}`,
            ...user,
        }
    }

    @Get('refresh_token')
    @Public()
    @UseGuards(RefreshTokenGuard)
    async refreshTokens(
        @Req() req,
        @Res({ passthrough: true }) res
    ): Promise<LoginSuccessResponse> {
        const { accessToken, refreshToken } = await this.authService.refreshTokens(
            req.user.sub,
            req.user.role,
            req.user.refreshToken
        )
        const data = { 
            username: req.user.username,
            userrole: req.user.role,
            modified_field: 'user login refresh token',
            modified_value: true
        }
        await this.dbiService.postUIConfigAuditLog(data);
        res.cookie('refreshToken', refreshToken, {
            sameSite: true,
            httpOnly: true,
            secure: true,
        })
        return {
            accessToken: accessToken,
            username: req.user.sub,
            role: req.user.role,
        }
    }

    // Potentially going to redesign how this works
    @Post('superauth')
    superauth(): string {
        return 'This action handles mfa and password expired'
    }
    @ApiOkResponse({ type: LoginSuccessResponse })
    @Get('authenticate-user-token')
    async authenticateUserToken(@Req() req): Promise<LoginSuccessResponse> {
        const response = this.authService.authUserToken(req.user)
        const data = { 
            modified_field: 'authentication token login',
            modified_value: true,
            username: req.user.username,
            userrole: req.user.role,
        }
        await this.dbiService.postUIConfigAuditLog(data);
        return response
    }
    @ApiOkResponse({ type: Logout })
    @Post('logout')
    @HttpCode(200)
    async logout(@Req() request: Request, @UserFromAccessToken() user: User): Promise<Logout> {
        const response = this.authService.logout(request)
        const data = { 
            modified_field: 'user logout',
            modified_value: true,
            username: user.username,
            userrole: user.role
        }
        await this.dbiService.postUIConfigAuditLog(data);
        return response
    }

    @Get('accessToken-test')
    accessTokenTest(): string {
        return 'the accessToken passed'
    }
}
