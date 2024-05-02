import { Inject, Injectable } from '@nestjs/common';
import { JwtService } from '@nestjs/jwt';
import { Cron, CronExpression } from '@nestjs/schedule';
import { APP_ENV_SERVICE, IAppEnvService } from 'src/environment/appEnv.interface';
import { IUsersService, USERS_SERVICE } from 'src/users/interfaces/users.service.interface';

@Injectable()
export class RefreshTokenService {
  constructor(
    private readonly jwtService: JwtService,
    @Inject(APP_ENV_SERVICE)
    private appEnvService: IAppEnvService,
    @Inject(USERS_SERVICE)
    private readonly usersService: IUsersService,
  ) {}

  private readonly refreshTokens: Set<string> = new Set();

  addRefreshToken(token: string) {
    this.refreshTokens.add(token);
  }

  invalidateRefreshToken(token: string): boolean {
    return this.refreshTokens.delete(token);
  }

  containsRefreshToken(token: string): boolean {
    return this.refreshTokens.has(token);
  }

  async validateRefreshToken(token: string): Promise<boolean> {
    try {
      const { userId, role } = await this.jwtService.verifyAsync(token);

      // check if user is still a valid user
      const user = await this.usersService.readById(userId);
      // check if jwt is in the list of valid JWTs
      return user?.role === role && this.containsRefreshToken(token);
    } catch (error) {
      return false;
    }
  }

  async createRefreshTokenFromUser(username: string, role: string): Promise<string> {
    const user = await this.usersService.readByUsername(username);
    const payload = { sub: username, userId: user.id, role: role, nonce: performance.now() };
    const refreshToken = this.jwtService.sign(payload, {
      expiresIn: this.appEnvService.getRefreshTokenTimeout(),
    });
    this.addRefreshToken(refreshToken);
    return refreshToken;
  }

  extractRefreshTokenFromRequest(req): string {
    let refreshToken = null;
    if (req && req.cookies) {
      refreshToken = req.cookies['refreshToken'];
    }
    return refreshToken;
  }

  @Cron(CronExpression.EVERY_MINUTE)
  garbageCollector() {
    this.refreshTokens.forEach(async (token) => {
      const isValid = await this.validateRefreshToken(token);

      if (!isValid) {
        this.invalidateRefreshToken(token);
      }
    });
  }
}
