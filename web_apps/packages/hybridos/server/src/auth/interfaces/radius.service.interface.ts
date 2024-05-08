export const RADIUS_SERVICE = 'RadiusService';
export interface IRadiusService {
  authenticate(username: string, password: string): Promise<string>;
}
