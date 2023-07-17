import { User } from 'src/users/dtos/user.dto';

export const PERMISSIONS_SERVICE = 'PermissionsService';
export const PERMISSIONS_FILE_READER_SERVICE = 'PermissionsFileReaderService';

export interface Permissions {
  version: number;
  roles: {
    [key: string]: Role;
  };
}

export interface Role {
  included: string;
  [key: string]: Permission | string;
}

export interface Permission {
  _accessLevel: number;
  [key: string]: Permission | number;
}

export interface PermissionsServiceInterface {
  ConfirmRoleAccess(user: User, permissionLevel: PermissionLevel, uri: string): boolean;
}

export enum PermissionLevel {
  READ = 'read',
  READ_WRITE = 'readWrite',
}

export enum PermissionFileFormat {
  LEGACY = 'Legacy',
  VERSION_2 = 'Version2',
}
