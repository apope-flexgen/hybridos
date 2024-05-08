import { User } from '../../../../shared/types/dtos/auth.dto';
import { PassExpCredentials } from '../dto/passExpCredentials.dto';

export const AUTH_SERVICE = 'AuthService';
export interface IAuthService {
  validateUser(username: string, plainTextPass: string): Promise<any>;
  login(user: User);
  loginMFA(username: string);
  logout(req);
  authUserToken(user: User);
  loginPassExp(passExpCredentials: PassExpCredentials);
  refreshTokens(username: string, role: string, refreshToken: string): Promise<any>;
}
