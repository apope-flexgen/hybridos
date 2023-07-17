import { Inject, Injectable } from '@nestjs/common';

import {
  Permissions,
  Permission,
  Role,
  PermissionsServiceInterface,
  PermissionLevel,
  PERMISSIONS_FILE_READER_SERVICE,
} from './interfaces/permission.interface';
import { User } from 'src/users/dtos/user.dto';
import { PermissionsFileReaderService } from './permissions.fileReader.service';
import { getBypassLevel, getNumericLevel } from './permissions.helper';

@Injectable()
export class PermissionsService implements PermissionsServiceInterface {
  // holds the perms object in memory
  permissions: Permissions;

  constructor(
    @Inject(PERMISSIONS_FILE_READER_SERVICE)
    private readonly permissionsFileReaderService: PermissionsFileReaderService,
  ) {
    this.permissions = permissionsFileReaderService.file;
  }

  // public method to check if a role has access to a uri;
  // returns true if access level is greater than requested
  // permissionLevel
  ConfirmRoleAccess = (user: User, permissionLevel: PermissionLevel, uri: string): boolean => {
    const [roleAccessLevel] = this.getAccessLevel(user.role, uri);
    const [usernameAccessLevel] = this.getAccessLevel(user.username, uri);
    const bypassLevel = getBypassLevel(user.role);

    const numericLevel = getNumericLevel(permissionLevel);
    return Math.max(roleAccessLevel, usernameAccessLevel, bypassLevel) >= numericLevel;
  };

  // returns the numeric access level of a role for a given uri
  getAccessLevel(userIdentifier: string, uri: string): number[] {
    // if the uri or role is empty, return 0, 0
    if (uri === '' || userIdentifier === '') {
      return [0, 0];
    }

    // if role doesn't exist, no access, no specificity
    if (!this.permissions.roles[userIdentifier]) {
      return [0, 0];
    }

    // extract the permissions for the given role and get access
    const roleObj = this.permissions.roles[userIdentifier];
    return this.accessHelper(uri, roleObj);
  }

  private accessHelper = (uri: string, roleObj: Role): number[] => {
    let curSpecificity = 0;
    let curAccessLevel = 0;

    // check the "/" case
    const permObj = roleObj[''] as Permission;
    curAccessLevel = permObj._accessLevel;

    // other cases
    const [dugAccessLevel, dugSpecificity] = this.digUri(roleObj, uri);
    if (dugSpecificity > curSpecificity) {
      curAccessLevel = dugAccessLevel;
      curSpecificity = dugSpecificity;
    }

    // get inherited perms next - these should only override if more specific
    const [inheritedAccessLevel, inheritedSpecificity] = this.getInheritedPerms(roleObj, uri);
    if (inheritedSpecificity > curSpecificity) {
      curAccessLevel = inheritedAccessLevel;
      curSpecificity = inheritedSpecificity;
    }

    return [curAccessLevel, curSpecificity];
  };

  // helper function to dig through a uri and return the access level/specificity
  private digUri = (roleObj: Role, uri: string): number[] => {
    const splitUri = uri.split('/');
    let permObj: Permission;
    let curAccessLevel = 0;
    let curSpecificity = 0;

    if (splitUri.length > 1 && splitUri[1] != '') {
      // first split - separate because this is digging into a Role, not a Permission
      if (roleObj[splitUri[1]]) {
        permObj = roleObj[splitUri[1]] as Permission;
        curAccessLevel = permObj._accessLevel;
        curSpecificity = 1;
      }

      // following cases - digging into a Permission
      const [dugAccessLevel, dugSpecificity] = this.digPermission(permObj, splitUri);
      if (dugSpecificity > curSpecificity) {
        curAccessLevel = dugAccessLevel;
        curSpecificity = dugSpecificity;
      }
    }

    return [curAccessLevel, curSpecificity];
  };

  // helper function to dig through a permission and return the access level/specificity
  private digPermission = (permObj: Permission, splitUri: string[]): number[] => {
    let curAccessLevel = 0;
    let curSpecificity = 0;

    // permObj will only exist if the first split was present,
    // so we can use it to check if we should continue to second split
    if (permObj) {
      for (let i = 2; i < splitUri.length; i++) {
        if ((splitUri[i] != '' && permObj[splitUri[i]]) || '*' in permObj) {
          permObj = (permObj[splitUri[i]] ?? permObj['*']) as Permission;
          curAccessLevel = permObj._accessLevel;
          curSpecificity = i;
        } else {
          break;
        }
      }
    }
    return [curAccessLevel, curSpecificity];
  };

  // helper function to get inherited permissions
  private getInheritedPerms = (roleObj: Role, uri: string): number[] => {
    let curAccessLevel = 0;
    let curSpecificity = 0;

    if (roleObj.included != '') {
      const [inheritedAccessLevel, inheritedSpecificity] = this.getAccessLevel(
        roleObj.included,
        uri,
      );
      if (inheritedSpecificity > curSpecificity) {
        curAccessLevel = inheritedAccessLevel;
        curSpecificity = inheritedSpecificity;
      }
    }

    return [curAccessLevel, curSpecificity];
  };
}
