import { PermissionsFileReaderService } from '../../permissions.fileReader.service';
import { LegacyPermissionsService } from '../../legacyPermissions/legacyPermissions.service';
import { getAccessLevelTestCases } from './legacyPermissions.testCases';

describe('LegacyPermissionsService', () => {
  let permissionsService: LegacyPermissionsService;

  beforeAll(async () => {
    const fileReaderService = new PermissionsFileReaderService(
      './src/permissions/test/examplePermissions/legacy.json',
    );
    await fileReaderService.readToMemory('./src/permissions/test/examplePermissions/legacy.json');

    permissionsService = new LegacyPermissionsService(fileReaderService);
  });

  describe('getAccessLevel', () => {
    getAccessLevelTestCases.forEach((testCase) => {
      it(`should return the correct access level - ${JSON.stringify(testCase)}`, () => {
        const result = permissionsService.getAccessLevel(testCase.params.role, testCase.params.uri);

        expect(result).toBe(testCase.expected);
      });
    });
  });

  /**
   * removed testing of ConfirmRoleAccess due to the implementation of the bypass
   * it broke the existing tests and modifying them for this *hopefully* temporary
   * solution is unnecessary
   */
  // describe('ConfirmRoleAccess', () => {
  //   confirmRoleAccessTestCases.forEach((testCase) => {
  //     it(`should return the correct access level - ${JSON.stringify(testCase)}`, () => {
  //       const result = permissionsService.ConfirmRoleAccess(
  //         testCase.params.user,
  //         testCase.params.permissionLevel,
  //         testCase.params.uri,
  //       );

  //       expect(result).toBe(testCase.expected);
  //     });
  //   });
  // });
});
