import { Inject, Injectable, OnApplicationBootstrap } from '@nestjs/common';
import { User } from 'src/users/dtos/user.dto';
import {
  PermissionLevel,
  PermissionsServiceInterface,
  PERMISSIONS_FILE_READER_SERVICE,
} from '../interfaces/permission.interface';
import { PermissionsFileReaderService } from '../permissions.fileReader.service';
import { getBypassLevel, getNumericLevel } from '../permissions.helper';
import { transformURI } from './legacyPermissions.helper';
import { LegacyRole, LegacyPermissions } from './legacyPermissions.types';

@Injectable()
export class LegacyPermissionsService implements PermissionsServiceInterface {
  // holds the perms object in memory
  permissions: LegacyRole[];

  constructor(
    @Inject(PERMISSIONS_FILE_READER_SERVICE)
    private readonly permissionsFileReaderService: PermissionsFileReaderService,
  ) {
    const file: LegacyPermissions = permissionsFileReaderService.file;
    this.permissions = file.authentication.permissions;
  }

  // public method to check if a role has access to a uri;
  // returns true if access level is greater than requested
  // permissionLevel
  ConfirmRoleAccess = (user: User, permissionLevel: PermissionLevel, uri: string): boolean => {
    const roleAccessLevel = this.getAccessLevel(user.role, uri);
    const usernameAccessLevel = this.getAccessLevel(user.username, uri);
    const bypassLevel = getBypassLevel(user.role);

    const numericLevel = getNumericLevel(permissionLevel);
    return Math.max(roleAccessLevel, usernameAccessLevel, bypassLevel) >= numericLevel;
  };

  getAccessLevel = (userIdentifier: string, uri: string): number => {
    const permissionsList = this.getPermissionsList(userIdentifier);
    // console.log('transforming URIs');
    const URIs: string[] = transformURI(uri);
    // console.log('transformed URIs', URIs);

    const accessLevels: number[] = URIs.map((uri): number => {
      if (permissionsList.includes(`${uri} ${PermissionLevel.READ_WRITE}`)) return 2;
      if (permissionsList.includes(`${uri} ${PermissionLevel.READ}`)) return 1;
      return 0;
    });

    return Math.max(...accessLevels);
  };

  getPermissionsList = (userIdentifier: string): string[] => {
    const rolePermissions = this.permissions.find((p) => p.roleOrUsername === userIdentifier);
    return rolePermissions?.access ?? [];
  };
}
