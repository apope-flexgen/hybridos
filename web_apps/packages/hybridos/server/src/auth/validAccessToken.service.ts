import { Inject, Injectable } from '@nestjs/common';
import { JwtService } from '@nestjs/jwt';
import { Cron, CronExpression } from '@nestjs/schedule';
import { IValidAccessTokenService } from 'src/auth/interfaces/validAccessToken.service.interface';
import { APP_ENV_SERVICE, IAppEnvService } from 'src/environment/appEnv.interface';
import { IUsersService, USERS_SERVICE } from 'src/users/interfaces/users.service.interface';

@Injectable()
export class ValidAccessTokenService implements IValidAccessTokenService {
  constructor(
    private readonly jwtService: JwtService,
    @Inject(APP_ENV_SERVICE)
    private appEnvService: IAppEnvService,
    @Inject(USERS_SERVICE)
    private readonly usersService: IUsersService,
  ) {}

  private readonly validJWTs: Set<string> = new Set();

  addAccessToken(accessToken: string) {
    this.validJWTs.add(accessToken);
  }

  invalidateAccessToken(accessToken: string): boolean {
    return this.validJWTs.delete(accessToken);
  }

  containsAccessToken(accessToken: string): boolean {
    return this.validJWTs.has(accessToken);
  }

  async validateAccessToken(accessToken: string): Promise<boolean> {
    try {
      const { userId, role } = await this.jwtService.verifyAsync(accessToken);

      // check if user is still a valid user
      const user = await this.usersService.readById(userId);
      // check if jwt is in the list of valid JWTs
      return user?.role === role && this.containsAccessToken(accessToken);
    } catch (error) {
      return false;
    }
  }

  async createAccessTokenFromUser(username: string, role: string): Promise<string> {
    const user = await this.usersService.readByUsername(username);
    const payload = {
      sub: username,
      userId: user.id,
      role: role,
      nonce: performance.now(),
    };
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
    this.validJWTs.forEach(async (accessToken) => {
      const isValid = await this.validateAccessToken(accessToken);

      if (!isValid) {
        this.invalidateAccessToken(accessToken);
      }
    });
  }

  extractUserDataFromToken(token: string) {
    return this.jwtService.decode(token);
  }
}
