import { Roles } from '../../../shared/types/api/Users/Users.types';
import { User } from '../users/dtos/user.dto';
import { PermissionFileFormat, PermissionLevel, Role } from './interfaces/permission.interface';
import { LegacyPermissionsService } from './legacyPermissions/legacyPermissions.service';
import { PermissionsFileReaderService } from './permissions.fileReader.service';
import { PermissionsService } from './permissions.service';

// helper function to get the numeric level of a permission
export const getNumericLevel = (level: PermissionLevel): number => {
  switch (level) {
    case PermissionLevel.READ:
      return 1;
    case PermissionLevel.READ_WRITE:
      return 2;
    default:
      return 0;
  }
};

export const getBypassLevel = (role: User['role']): number => {
  switch (role) {
    case Roles.Developer:
    case Roles.Admin:
    case Roles.User:
      return 2;
    case Roles.Observer:
      return 1;
    case Roles.Rest:
    case Roles.RestReadWrite:
    default:
      return 0;
  }
};

export const PermissionsServiceFactory = async (
  fileReaderService: PermissionsFileReaderService,
): Promise<any> => {
  await fileReaderService.init();
  switch (fileReaderService.fileFormat) {
    case PermissionFileFormat.LEGACY:
      return new LegacyPermissionsService(fileReaderService);
    case PermissionFileFormat.VERSION_2:
      return new PermissionsService(fileReaderService);
    default:
      throw new Error('invalid permissions file format');
  }
};
