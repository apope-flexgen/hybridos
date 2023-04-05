export const VALID_ACCESS_TOKEN_SERVICE = 'ValidJWTService';
export interface IValidJWTService {
  addAccessToken(accessToken: string);
  invalidateAccessToken(accessToken: string): boolean;
  containsAccessToken(accessToken: string): boolean;
  createAccessTokenFromUser(username: string, role: string): string;
  extractAccessTokenFromRequest(req): string;
  garbageCollector();
}
