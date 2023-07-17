// TODO: fix this rule
/* eslint-disable max-lines, max-statements */
import { PermissionsService } from '../permissions.service';
import { PermissionsFileReaderService } from '../permissions.fileReader.service';
import { getAccessLevelWildcardTestCases } from './permissions.testCases';

describe('PermissionsService', () => {
  let permissionsService: PermissionsService;

  beforeEach(async () => {
    const fileReaderService = new PermissionsFileReaderService(
      './src/permissions/test/examplePermissions/testPerms.json',
    );
    await fileReaderService.readToMemory(
      './src/permissions/test/examplePermissions/testPerms.json',
    );

    permissionsService = new PermissionsService(fileReaderService);
  });

  describe('getAccessLevel', () => {
    describe('version 2', () => {
      // >>>>> INVALID/EMPTY CASES <<<<<
      it('should return [0, 0] for an empty uri', () => {
        const [accessLevel, specificity] = permissionsService.getAccessLevel('admin', '');
        expect(accessLevel).toBe(0);
        expect(specificity).toBe(0);
      });

      it('should return default access ([2, 0] for admin) for an invalid uri', () => {
        const [accessLevel, specificity] = permissionsService.getAccessLevel('admin', 'invalid');
        expect(accessLevel).toBe(2);
        expect(specificity).toBe(0);
      });

      it('should return [0, 0] for an invalid role', () => {
        const [accessLevel, specificity] = permissionsService.getAccessLevel('invalid', '/assets');
        expect(accessLevel).toBe(0);
        expect(specificity).toBe(0);
      });

      it('should return [0, 0] for an invalid role and uri', () => {
        const [accessLevel, specificity] = permissionsService.getAccessLevel('invalid', 'invalid');
        expect(accessLevel).toBe(0);
        expect(specificity).toBe(0);
      });

      it('should return [0, 0] for an empty role and uri', () => {
        const [accessLevel, specificity] = permissionsService.getAccessLevel('', '');
        expect(accessLevel).toBe(0);
        expect(specificity).toBe(0);
      });

      it('should return [0, 0] for an empty role and valid uri', () => {
        const [accessLevel, specificity] = permissionsService.getAccessLevel('', '/assets');
        expect(accessLevel).toBe(0);
        expect(specificity).toBe(0);
      });

      it('should return [0, 0] for an empty role and invalid uri', () => {
        const [accessLevel, specificity] = permissionsService.getAccessLevel('', 'invalid');
        expect(accessLevel).toBe(0);
        expect(specificity).toBe(0);
      });

      it('should return [0, 0] for an invalid role and empty uri', () => {
        const [accessLevel, specificity] = permissionsService.getAccessLevel('invalid', '');
        expect(accessLevel).toBe(0);
        expect(specificity).toBe(0);
      });

      // >>>>> VALID CASES <<<<<
      it('should return [2, 0] for admin and /', () => {
        const [accessLevel, specificity] = permissionsService.getAccessLevel('admin', '/');
        expect(accessLevel).toBe(2);
        expect(specificity).toBe(0);
      });

      it('should return [2, 0] for admin and /assets', () => {
        const [accessLevel, specificity] = permissionsService.getAccessLevel('admin', '/assets');
        expect(accessLevel).toBe(2);
        expect(specificity).toBe(0);
      });

      it('should return [2, 0] for admin and /assets/1', () => {
        const [accessLevel, specificity] = permissionsService.getAccessLevel('admin', '/assets/1');
        expect(accessLevel).toBe(2);
        expect(specificity).toBe(0);
      });

      it('should return [0, 0] for user and /', () => {
        const [accessLevel, specificity] = permissionsService.getAccessLevel('user', '/');
        expect(accessLevel).toBe(0);
        expect(specificity).toBe(0);
      });

      it('should return [0, 1] for user and /assets', () => {
        const [accessLevel, specificity] = permissionsService.getAccessLevel('user', '/assets');
        expect(accessLevel).toBe(0);
        expect(specificity).toBe(1);
      });

      it('should return [1, 2] for user and /assets/ess', () => {
        const [accessLevel, specificity] = permissionsService.getAccessLevel('user', '/assets/ess');
        expect(accessLevel).toBe(1);
        expect(specificity).toBe(2);
      });

      it('should return [2, 3] for user and /assets/ess/1', () => {
        const [accessLevel, specificity] = permissionsService.getAccessLevel(
          'user',
          '/assets/ess/1',
        );
        expect(accessLevel).toBe(2);
        expect(specificity).toBe(3);
      });

      it('should return [2, 3] for user and /assets/ess/1/2', () => {
        const [accessLevel, specificity] = permissionsService.getAccessLevel(
          'user',
          '/assets/ess/1/2',
        );
        expect(accessLevel).toBe(2);
        expect(specificity).toBe(3);
      });

      it('should return [0, 0] for inheritsFromUser and /', () => {
        const [accessLevel, specificity] = permissionsService.getAccessLevel(
          'inheritsFromUser',
          '/',
        );
        expect(accessLevel).toBe(0);
        expect(specificity).toBe(0);
      });

      it('should return [0, 1] for inheritsFromUser and /assets', () => {
        const [accessLevel, specificity] = permissionsService.getAccessLevel(
          'inheritsFromUser',
          '/assets',
        );
        expect(accessLevel).toBe(0);
        expect(specificity).toBe(1);
      });

      it('should return [1, 2] for inheritsFromUser and /assets/ess', () => {
        const [accessLevel, specificity] = permissionsService.getAccessLevel(
          'inheritsFromUser',
          '/assets/ess',
        );
        expect(accessLevel).toBe(1);
        expect(specificity).toBe(2);
      });

      it('should return [2, 3] for inheritsFromUser and /assets/ess/1', () => {
        const [accessLevel, specificity] = permissionsService.getAccessLevel(
          'inheritsFromUser',
          '/assets/ess/1',
        );
        expect(accessLevel).toBe(2);
        expect(specificity).toBe(3);
      });

      it('should return [2, 3] for inheritsFromUser and /assets/ess/1/2', () => {
        const [accessLevel, specificity] = permissionsService.getAccessLevel(
          'inheritsFromUser',
          '/assets/ess/1/2',
        );
        expect(accessLevel).toBe(2);
        expect(specificity).toBe(3);
      });

      it('should return [0, 0] for inheritsFromInheritsFromUser and /', () => {
        const [accessLevel, specificity] = permissionsService.getAccessLevel(
          'inheritsFromInheritsFromUser',
          '/',
        );
        expect(accessLevel).toBe(0);
        expect(specificity).toBe(0);
      });

      it('should return [0, 1] for inheritsFromInheritsFromUser and /assets', () => {
        const [accessLevel, specificity] = permissionsService.getAccessLevel(
          'inheritsFromInheritsFromUser',
          '/assets',
        );
        expect(accessLevel).toBe(0);
        expect(specificity).toBe(1);
      });

      it('should return [1, 2] for inheritsFromInheritsFromUser and /assets/ess', () => {
        const [accessLevel, specificity] = permissionsService.getAccessLevel(
          'inheritsFromInheritsFromUser',
          '/assets/ess',
        );
        expect(accessLevel).toBe(1);
        expect(specificity).toBe(2);
      });

      it('should return [2, 3] for inheritsFromInheritsFromUser and /assets/ess/1', () => {
        const [accessLevel, specificity] = permissionsService.getAccessLevel(
          'inheritsFromInheritsFromUser',
          '/assets/ess/1',
        );
        expect(accessLevel).toBe(2);
        expect(specificity).toBe(3);
      });

      it('should return [2, 3] for inheritsFromInheritsFromUser and /assets/ess/1/2', () => {
        const [accessLevel, specificity] = permissionsService.getAccessLevel(
          'inheritsFromInheritsFromUser',
          '/assets/ess/1/2',
        );
        expect(accessLevel).toBe(2);
        expect(specificity).toBe(3);
      });

      it('should return [0, 0] for inheritsFromUserWithMods and /', () => {
        const [accessLevel, specificity] = permissionsService.getAccessLevel(
          'inheritsFromUserWithMods',
          '/',
        );
        expect(accessLevel).toBe(0);
        expect(specificity).toBe(0);
      });

      it('should return [1, 1] for inheritsFromUserWithMods and /assets', () => {
        const [accessLevel, specificity] = permissionsService.getAccessLevel(
          'inheritsFromUserWithMods',
          '/assets',
        );
        expect(accessLevel).toBe(1);
        expect(specificity).toBe(1);
      });

      it('should return [2, 2] for inheritsFromUserWithMods and /assets/ess', () => {
        const [accessLevel, specificity] = permissionsService.getAccessLevel(
          'inheritsFromUserWithMods',
          '/assets/ess',
        );
        expect(accessLevel).toBe(2);
        expect(specificity).toBe(2);
      });

      it('should return [0, 3] for inheritsFromUserWithMods and /assets/ess/1', () => {
        const [accessLevel, specificity] = permissionsService.getAccessLevel(
          'inheritsFromUserWithMods',
          '/assets/ess/1',
        );
        expect(accessLevel).toBe(0);
        expect(specificity).toBe(3);
      });
    });

    describe('wildcards', () => {
      getAccessLevelWildcardTestCases.forEach((testCase) => {
        it(`should return the correct access level - ${JSON.stringify(testCase)}`, () => {
          const result = permissionsService.getAccessLevel(
            testCase.params.role,
            testCase.params.uri,
          );

          expect(result).toEqual(testCase.expected);
        });
      });
    });
  });
});
