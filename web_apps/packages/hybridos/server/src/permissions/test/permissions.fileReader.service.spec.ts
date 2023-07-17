import * as fs from 'fs';
import { PermissionFileFormat } from '../interfaces/permission.interface';
import { PermissionsFileReaderService } from '../permissions.fileReader.service';

describe('PermissionsFileReaderService', () => {
  let fileReaderService: PermissionsFileReaderService;

  beforeEach(() => {
    fileReaderService = new PermissionsFileReaderService('');
  });

  // Tests both readToMemory and readPermissionsFromFile
  describe('readToMemory', () => {
    it('should read the permissions file to memory', async () => {
      await fileReaderService.readToMemory(
        './src/permissions/test/examplePermissions/testPerms.json',
      );
      expect(fileReaderService.fileFormat).toBe(PermissionFileFormat.VERSION_2);
      expect(fileReaderService.file).toBeDefined();

      const file = fs.readFileSync(
        './src/permissions/test/examplePermissions/testPerms.json',
        'utf8',
      );
      const permissions = JSON.parse(file);
      expect(fileReaderService.file).toMatchObject(permissions);
    });

    it('should read the legacy permissions file to memory', async () => {
      await fileReaderService.readToMemory('./src/permissions/test/examplePermissions/legacy.json');
      expect(fileReaderService.fileFormat).toBe(PermissionFileFormat.LEGACY);
      expect(fileReaderService.file).toBeDefined();

      const file = fs.readFileSync('./src/permissions/test/examplePermissions/legacy.json', 'utf8');
      const permissions = JSON.parse(file);
      expect(fileReaderService.file).toMatchObject(permissions);
    });

    describe('invalid files', () => {
      it('should throw an error with a blank file', async () => {
        await expect(
          fileReaderService.readToMemory(
            './src/permissions/test/examplePermissions/invalid/blankFile.json',
          ),
        ).rejects.toThrowError();

        expect(fileReaderService.fileFormat).toBeUndefined();
        expect(fileReaderService.file).toBeUndefined();
      });
      it('should throw an error with a missing version field', async () => {
        await expect(
          fileReaderService.readToMemory(
            './src/permissions/test/examplePermissions/invalid/missingVersionField.json',
          ),
        ).rejects.toThrowError();

        expect(fileReaderService.fileFormat).toBeUndefined();
        expect(fileReaderService.file).toBeUndefined();
      });
      it('should throw an error with an empty object', async () => {
        await expect(
          fileReaderService.readToMemory(
            './src/permissions/test/examplePermissions/invalid/emptyObject.json',
          ),
        ).rejects.toThrowError();

        expect(fileReaderService.fileFormat).toBeUndefined();
        expect(fileReaderService.file).toBeUndefined();
      });
    });
  });
});
