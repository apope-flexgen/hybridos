import { Inject, Injectable } from '@nestjs/common';
import { compare } from 'bcryptjs';

import { LogoutResponse, User } from '../../../shared/types/dtos/auth.dto';
import { PassExpCredentials } from './dto/passExpCredentials.dto';
import { IUsersService, USERS_SERVICE } from '../users/interfaces/users.service.interface';
import { UnchangedPasswordException } from './exceptions/unchangedPassword.exception';
import { IMfaService, MFA_SERVICE } from './interfaces/mfa.service.interface';
import { IValidJWTService, VALID_JWT_SERVICE } from './interfaces/validJWT.service.interface';
import { RefreshTokenService } from './refreshTokenService';

@Injectable()
export class AuthService {
  constructor(
    @Inject(USERS_SERVICE) private readonly usersService: IUsersService,
    @Inject(MFA_SERVICE)
    private readonly mfaService: IMfaService,
    @Inject(VALID_JWT_SERVICE)
    private readonly validJwtService: IValidJWTService,
    private readonly refreshTokenService: RefreshTokenService,
  ) {}
  async validateUser(username: string, plainTextPass: string): Promise<User> {
    const { password, ...user } = await this.usersService.readByUsername(username);

    const correctPassword = await compare(plainTextPass, password);

    return correctPassword ? (user as User) : null;
  }

  async login(user: User) {
    const accessToken = this.validJwtService.createAccessTokenFromUser(user.username, user.role);

    const refreshToken = this.refreshTokenService.createRefreshTokenFromUser(
      user.username,
      user.role,
    );

    return {
      username: user.username,
      role: user.role,
      accessToken: accessToken,
      refreshToken: refreshToken,
    };
  }

  async loginMFA(username: string) {
    const user = await this.usersService.readByUsername(username);

    // update user's mfaEnabled field
    const updateSuccessful = await this.usersService.enableMfa(user.id);
    if (!updateSuccessful) {
      throw new Error('unsuccessful user update');
    }

    const response = await this.login(user as User);
    return response;
  }

  logout(req): LogoutResponse {
    const accessToken = this.validJwtService.extractAccessTokenFromRequest(req);
    const refreshToken = this.refreshTokenService.extractRefreshTokenFromRequest(req);

    const removedAccessToken = this.validJwtService.invalidateAccessToken(accessToken);
    const removedRefreshToken = this.refreshTokenService.invalidateRefreshToken(refreshToken);

    if (!removedAccessToken || !removedRefreshToken) {
      throw new Error('unable to invalidate jwts');
    }
    return { message: 'user has been logged out' };
  }

  authUserToken(user: User) {
    return {
      username: user.username,
      role: user.role,
    };
  }

  async refreshTokens(username: string, role: string, oldRefreshToken: string) {
    this.refreshTokenService.invalidateRefreshToken(oldRefreshToken);

    const newAccessToken = this.validJwtService.createAccessTokenFromUser(username, role);

    const newRefreshToken = this.refreshTokenService.createRefreshTokenFromUser(username, role);

    return {
      refreshToken: newRefreshToken,
      accessToken: `Bearer ${newAccessToken}`,
    };
  }

  async loginPassExp(passExpCredentials: PassExpCredentials) {
    const user = await this.usersService.readByUsername(passExpCredentials.username);

    // check if user updated their password
    const passwordsMatch = await compare(passExpCredentials.updatedPassword, user.password);
    if (passwordsMatch) {
      throw new UnchangedPasswordException();
    }

    // update user's password
    await this.usersService.update(user.id, {
      password: passExpCredentials.updatedPassword,
      username: user.username,
      role: user.role,
    });

    // FIXME: "as User" is a bandaid fix for bad typing here
    await this.mfaService.checkIfSiteMfaEnabled(user as User);

    const response = await this.login(user as User);
    return response;
  }
}
