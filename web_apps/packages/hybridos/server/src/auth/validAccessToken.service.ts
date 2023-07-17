import { Injectable } from '@nestjs/common';
import { JwtService } from '@nestjs/jwt';
import { Cron, CronExpression } from '@nestjs/schedule';
import { AppEnvService } from '../environment/appEnv.service';

@Injectable()
export class ValidAccessTokenService {
  constructor(
    private readonly jwtService: JwtService,
    private readonly appEnvService: AppEnvService,
  ) {}

  private readonly validAccessTokens: Set<string> = new Set();

  addAccessToken(accessToken: string) {
    this.validAccessTokens.add(accessToken);
  }

  invalidateAccessToken(accessToken: string): boolean {
    return this.validAccessTokens.delete(accessToken);
  }

  containsAccessToken(accessToken: string): boolean {
    return this.validAccessTokens.has(accessToken);
  }

  createAccessTokenFromUser(username: string, role: string): string {
    const payload = { sub: username, role: role, nonce: performance.now() };
    const accessToken = this.jwtService.sign(payload, {
      expiresIn: this.appEnvService.getAccessTokenTimeout(),
    });
    this.addAccessToken(accessToken);
    return accessToken;
  }

  extractAccessTokenFromRequest(req): string {
    let token = null;
    if (req && req.headers) {
      token = req.headers['authorization'].split(' ')[1];
    }
    return token;
  }

  @Cron(CronExpression.EVERY_MINUTE)
  garbageCollector() {
    this.validAccessTokens.forEach(async (accessToken) => {
      try {
        await this.jwtService.verifyAsync(accessToken);
      } catch (TokenExpiredError) {
        this.validAccessTokens.delete(accessToken);
      }
    });
  }
}
