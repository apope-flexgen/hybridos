import { Injectable } from '@nestjs/common';
import { JwtService } from '@nestjs/jwt';
import { AppEnvService } from '../environment/appEnv.service';
import { Cron, CronExpression } from '@nestjs/schedule';

@Injectable()
export class RefreshTokenService {
  constructor(
    private readonly jwtService: JwtService,
    private readonly appEnvService: AppEnvService,
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

  createRefreshTokenFromUser(username: string, role: string): string {
    const payload = { sub: username, role: role, nonce: performance.now() };
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
    this.refreshTokens.forEach(async (accessToken) => {
      try {
        await this.jwtService.verifyAsync(accessToken);
      } catch (TokenExpiredError) {
        this.refreshTokens.delete(accessToken);
      }
    });
  }
}
