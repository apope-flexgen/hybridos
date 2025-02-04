import { UserResponse } from '../responses/user.response';

export const DEFAULT_USER_SERVICE = 'DefaultUserService';
export interface IDefaultUserService {
  onApplicationBootstrap();
  createDefaultUser(): Promise<UserResponse>;
  deleteDefaultUser(): Promise<boolean>;
  getDefaultUser(): UserResponse | null;
  setDefaultUser(defaultUser: UserResponse | null): void;
  getDefaultUserID(): string;
}
