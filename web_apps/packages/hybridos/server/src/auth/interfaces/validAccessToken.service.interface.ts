export const VALID_ACCESS_TOKEN_SERVICE = 'ValidAccessTokenService';
export interface IValidAccessTokenService {
  addAccessToken(accessToken: string);
  invalidateAccessToken(accessToken: string): boolean;
  containsAccessToken(accessToken: string): boolean;
  validateAccessToken(accessToken: string): Promise<boolean>;
  createAccessTokenFromUser(username: string, role: string): Promise<string>;
  extractAccessTokenFromRequest(req): string;
  garbageCollector();
  extractUserDataFromToken(token: string);
}
