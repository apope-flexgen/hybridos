import { Inject, Injectable } from '@nestjs/common';
import * as fs from 'fs';
import * as path from 'path';
import { PermissionsFileError } from './exceptions/PermissionsFileError.exception';
import { PermissionFileFormat } from './interfaces/permission.interface';

@Injectable()
export class PermissionsFileReaderService {
  fileFormat: PermissionFileFormat;
  file: any;

  constructor(
    @Inject('WEB_SERVER_CONFIG_DIRECTORY_PATH')
    private readonly webServerConfigDirectoryPath: string,
  ) {}

  async init() {
    const filePath = path.resolve(this.webServerConfigDirectoryPath, 'permissions.json');
    // read file on init
    await this.readToMemory(filePath);
  }

  // stores perms in memory, sets initialized flag to true,
  // catches errors from reading file
  readToMemory = async (filePath: string): Promise<boolean> => {
    try {
      const { file, format } = await this.readPermissionsFromFile(filePath);
      this.file = file;
      this.fileFormat = format;
      return true;
    } catch (e) {
      throw new PermissionsFileError(e);
    }
  };

  // helper function, reads the permissions file
  private readPermissionsFromFile = async (
    filePath: string,
  ): Promise<{ file: Permissions; format: PermissionFileFormat }> => {
    const file = fs.readFileSync(filePath, 'utf8');
    const permissionsFile = JSON.parse(file);

    if (permissionsFile?.version === 2) {
      return { file: permissionsFile, format: PermissionFileFormat.VERSION_2 };
    } else if (permissionsFile?.hasOwnProperty('authentication')) {
      return { file: permissionsFile, format: PermissionFileFormat.LEGACY };
    } else {
      throw new Error('Invalid permissions.json format');
    }
  };
}
