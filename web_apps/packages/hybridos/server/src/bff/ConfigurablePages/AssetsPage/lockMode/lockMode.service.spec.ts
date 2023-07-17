/* eslint-disable max-nested-callbacks */
import { Test, TestingModule } from '@nestjs/testing';
import { DBI_SERVICE, DBI_URIs, IDBIService } from '../../../../dbi/dbi.interface';
import { LockModeService } from './lockMode.service';

const CATEGORY = 'ess';
const ASSET_ID = 'ess_1';
const ASSET_URI = `/assets/${CATEGORY}/${ASSET_ID}`;
const LOCKED = true;
const UNLOCKED = false;
const USERNAME = 'username';
const OTHER_USERNAME = 'otherUsername';
const ROLE_HAS_ACCESS = true;
const ROLE_NO_ACCESS = false;

const UNLOCKED_ASSET = {
  [CATEGORY]: {
    [ASSET_ID]: {
      value: false,
    },
  },
};

// const UNLOCKED_ASSET = {
//   assets: {
//     [CATEGORY]: {
//       asset_instances: [
//         {
//           id: ASSET_ID,
//           maint_mode_lockout: {
//             value: false,
//           },
//         },
//       ],
//     },
//   },
// };

const LOCKED_ASSET = (username: string) => {
  return {
    [CATEGORY]: {
      [ASSET_ID]: {
        value: true,
        username: username,
      },
    },
  };
};

// const LOCKED_ASSET = (username: string) => {
//   return {
//     assets: {
//       [CATEGORY]: {
//         asset_instances: [
//           {
//             id: ASSET_ID,
//             maint_mode_lockout: {
//               value: true,
//               username: username,
//             },
//           },
//         ],
//       },
//     },
//   };
// };

const LOCKED_BY_USERNAME = LOCKED_ASSET(USERNAME);
const LOCKED_BY_OTHER = LOCKED_ASSET(OTHER_USERNAME);

describe('LockModeService', () => {
  let lockModeService: LockModeService;
  let dbiService: IDBIService;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [
        LockModeService,
        {
          provide: DBI_SERVICE,
          useValue: {
            getFromDBI: jest.fn(),
            postToDBI: jest.fn(),
          },
        },
      ],
    }).compile();

    lockModeService = module.get<LockModeService>(LockModeService);
    dbiService = module.get<IDBIService>(DBI_SERVICE);
  });

  it('should be defined', () => {
    expect(lockModeService).toBeDefined();
    expect(lockModeService.lockModeStateMap).toStrictEqual({});
  });

  describe('test setStateForURI', () => {
    describe('URI is not in map', () => {
      it('URI not in map', () => {
        expect(lockModeService.lockModeStateMap).not.toHaveProperty(ASSET_URI);
      });

      it('should initialize map object with hasChanged true', () => {
        lockModeService.setStateForURI(ASSET_URI, UNLOCKED);

        expect(lockModeService.lockModeStateMap[ASSET_URI]).toStrictEqual({
          currentState: UNLOCKED,
          hasChanged: true,
        });
      });
    });

    describe('URI is in map', () => {
      describe('asset is locked', () => {
        beforeEach(() => {
          lockModeService.setStateForURI(ASSET_URI, LOCKED);
        });

        it('URI is in map with asset locked', () => {
          expect(lockModeService.lockModeStateMap).toHaveProperty(ASSET_URI);
          expect(lockModeService.lockModeStateMap[ASSET_URI]).toStrictEqual({
            currentState: LOCKED,
            hasChanged: true,
          });
        });

        it('state is updated to unlocked and hasChanged is true', () => {
          lockModeService.setStateForURI(ASSET_URI, UNLOCKED);

          expect(lockModeService.lockModeStateMap[ASSET_URI]).toStrictEqual({
            currentState: UNLOCKED,
            hasChanged: true,
          });
        });

        it('state remains locked and hasChanged is false', () => {
          lockModeService.setStateForURI(ASSET_URI, LOCKED);

          expect(lockModeService.lockModeStateMap[ASSET_URI]).toStrictEqual({
            currentState: LOCKED,
            hasChanged: true,
          });
        });

        describe('asset has username attached', () => {
          beforeEach(() => {
            lockModeService.lockModeStateMap[ASSET_URI].username = USERNAME;
          });

          it('should have username attached', () => {
            expect(lockModeService.lockModeStateMap[ASSET_URI]).toStrictEqual({
              currentState: LOCKED,
              hasChanged: true,
              username: USERNAME,
            });
          });

          describe('should still have username attached', () => {
            it('locked => locked', () => {
              lockModeService.setStateForURI(ASSET_URI, LOCKED);

              expect(lockModeService.lockModeStateMap[ASSET_URI]).toStrictEqual({
                currentState: LOCKED,
                hasChanged: true,
                username: USERNAME,
              });
            });

            it('locked => unlocked', () => {
              lockModeService.setStateForURI(ASSET_URI, UNLOCKED);

              expect(lockModeService.lockModeStateMap[ASSET_URI]).toStrictEqual({
                currentState: UNLOCKED,
                hasChanged: true,
                username: USERNAME,
              });
            });
          });
        });
      });

      describe('asset is unlocked', () => {
        beforeEach(() => {
          lockModeService.setStateForURI(ASSET_URI, UNLOCKED);
        });

        it('URI is in map with asset unlocked', () => {
          expect(lockModeService.lockModeStateMap).toHaveProperty(ASSET_URI);
          expect(lockModeService.lockModeStateMap[ASSET_URI]).toStrictEqual({
            currentState: UNLOCKED,
            hasChanged: true,
          });
        });

        it('state is updated to locked and hasChanged is true', () => {
          lockModeService.setStateForURI(ASSET_URI, LOCKED);

          expect(lockModeService.lockModeStateMap[ASSET_URI]).toStrictEqual({
            currentState: LOCKED,
            hasChanged: true,
          });
        });

        it('state remains unlocked and hasChanged is false', () => {
          lockModeService.setStateForURI(ASSET_URI, UNLOCKED);

          expect(lockModeService.lockModeStateMap[ASSET_URI]).toStrictEqual({
            currentState: UNLOCKED,
            hasChanged: true,
          });
        });

        describe('asset has username attached', () => {
          beforeEach(() => {
            lockModeService.lockModeStateMap[ASSET_URI].username = USERNAME;
          });

          it('should have username attached', () => {
            expect(lockModeService.lockModeStateMap[ASSET_URI]).toStrictEqual({
              currentState: UNLOCKED,
              hasChanged: true,
              username: USERNAME,
            });
          });

          describe('should still have username attached', () => {
            it('unlocked => locked', () => {
              lockModeService.setStateForURI(ASSET_URI, LOCKED);

              expect(lockModeService.lockModeStateMap[ASSET_URI]).toStrictEqual({
                currentState: LOCKED,
                hasChanged: true,
                username: USERNAME,
              });
            });

            it('unlocked => unlocked', () => {
              lockModeService.setStateForURI(ASSET_URI, UNLOCKED);

              expect(lockModeService.lockModeStateMap[ASSET_URI]).toStrictEqual({
                currentState: UNLOCKED,
                hasChanged: true,
                username: USERNAME,
              });
            });
          });
        });
      });

      describe('hasChanged is false', () => {
        beforeEach(() => {
          lockModeService.setStateForURI(ASSET_URI, LOCKED);
          lockModeService.lockModeStateMap[ASSET_URI].hasChanged = false;
        });

        it('should keep hasChanged false', () => {
          lockModeService.setStateForURI(ASSET_URI, LOCKED);

          expect(lockModeService.lockModeStateMap[ASSET_URI].hasChanged).toBe(false);
        });
      });
    });
  });

  describe('test getLockModeObject', () => {
    describe('URI is not in map', () => {
      it('URI not in map', () => {
        expect(lockModeService.lockModeStateMap).not.toHaveProperty(ASSET_URI);
      });

      it('should return undefined for username and access level', async () => {
        expect(
          await lockModeService.getLockModeObject(ASSET_URI, USERNAME, ROLE_HAS_ACCESS),
        ).toStrictEqual({
          username: undefined,
          lockModeButtonEnabled: undefined,
          isLocked: undefined,
        });
      });
    });

    describe('URI is in map', () => {
      describe('asset is locked', () => {
        beforeEach(() => {
          lockModeService.setStateForURI(ASSET_URI, LOCKED);
        });

        describe('by different user', () => {
          beforeEach(() => {
            lockModeService.lockModeStateMap[ASSET_URI].username = OTHER_USERNAME;
            jest
              .spyOn(dbiService, 'getFromDBI')
              .mockImplementation(() => Promise.resolve(LOCKED_BY_OTHER));
          });

          describe('state has changed', () => {
            it('hasChanged should be true', () => {
              expect(lockModeService.lockModeStateMap[ASSET_URI].hasChanged).toBe(true);
            });

            it('should return other username and no access', async () => {
              expect(
                await lockModeService.getLockModeObject(ASSET_URI, USERNAME, ROLE_HAS_ACCESS),
              ).toStrictEqual({
                username: OTHER_USERNAME,
                lockModeButtonEnabled: false,
                isLocked: true,
              });
            });
          });

          describe('state has not changed', () => {
            beforeEach(() => {
              lockModeService.lockModeStateMap[ASSET_URI].hasChanged = false;
            });

            it('hasChanged should be false', () => {
              expect(lockModeService.lockModeStateMap[ASSET_URI].hasChanged).toBe(false);
            });

            it('should return other username and no access', async () => {
              expect(
                await lockModeService.getLockModeObject(ASSET_URI, USERNAME, ROLE_HAS_ACCESS),
              ).toStrictEqual({
                username: OTHER_USERNAME,
                lockModeButtonEnabled: false,
                isLocked: true,
              });
            });
          });
        });

        describe('by requesting user', () => {
          beforeEach(() => {
            lockModeService.lockModeStateMap[ASSET_URI].username = USERNAME;
            jest
              .spyOn(dbiService, 'getFromDBI')
              .mockImplementation(() => Promise.resolve(LOCKED_BY_USERNAME));
          });
          describe('state has changed', () => {
            it('hasChanged should be true', () => {
              expect(lockModeService.lockModeStateMap[ASSET_URI].hasChanged).toBe(true);
            });

            it('should return username and access', async () => {
              expect(
                await lockModeService.getLockModeObject(ASSET_URI, USERNAME, ROLE_HAS_ACCESS),
              ).toStrictEqual({
                username: USERNAME,
                lockModeButtonEnabled: true,
                isLocked: true,
              });
            });
          });

          describe('state has not changed', () => {
            beforeEach(() => {
              lockModeService.lockModeStateMap[ASSET_URI].hasChanged = false;
            });

            it('hasChanged should be false', () => {
              expect(lockModeService.lockModeStateMap[ASSET_URI].hasChanged).toBe(false);
            });

            it('should return username and access', async () => {
              expect(
                await lockModeService.getLockModeObject(ASSET_URI, USERNAME, ROLE_HAS_ACCESS),
              ).toStrictEqual({
                username: USERNAME,
                lockModeButtonEnabled: true,
                isLocked: true,
              });
            });
          });
        });
      });

      describe('asset is unlocked', () => {
        beforeEach(() => {
          lockModeService.setStateForURI(ASSET_URI, UNLOCKED);

          jest
            .spyOn(dbiService, 'getFromDBI')
            .mockImplementation(() => Promise.resolve(UNLOCKED_ASSET));
        });

        describe('hasChanged is true', () => {
          it('hasChanged should be true', () => {
            expect(lockModeService.lockModeStateMap[ASSET_URI].hasChanged).toBe(true);
          });

          it('should return undefined for username and access level', async () => {
            expect(
              await lockModeService.getLockModeObject(ASSET_URI, USERNAME, ROLE_HAS_ACCESS),
            ).toStrictEqual({
              username: undefined,
              lockModeButtonEnabled: undefined,
              isLocked: false,
            });
          });
        });

        describe('hasChanged is false', () => {
          beforeEach(() => {
            lockModeService.lockModeStateMap[ASSET_URI].hasChanged = false;
          });

          it('hasChanged should be false', () => {
            expect(lockModeService.lockModeStateMap[ASSET_URI].hasChanged).toBe(false);
          });

          it('should return undefined for username and access level', async () => {
            expect(
              await lockModeService.getLockModeObject(ASSET_URI, USERNAME, ROLE_HAS_ACCESS),
            ).toStrictEqual({
              username: undefined,
              lockModeButtonEnabled: undefined,
              isLocked: false,
            });
          });
        });
      });

      describe('user role access', () => {
        describe('user has no access', () => {
          describe('asset locked by other user', () => {
            beforeEach(() => {
              lockModeService.setStateForURI(ASSET_URI, LOCKED);
              lockModeService.lockModeStateMap[ASSET_URI].username = OTHER_USERNAME;
              jest
                .spyOn(dbiService, 'getFromDBI')
                .mockImplementation(() => Promise.resolve(LOCKED_BY_OTHER));
            });

            it('lockModeButtonEnabled should be false', async () => {
              expect(
                await lockModeService.getLockModeObject(ASSET_URI, USERNAME, ROLE_NO_ACCESS),
              ).toStrictEqual({
                username: OTHER_USERNAME,
                lockModeButtonEnabled: false,
                isLocked: true,
              });
            });
          });

          describe('asset unlocked', () => {
            beforeEach(() => {
              lockModeService.setStateForURI(ASSET_URI, UNLOCKED);
              jest
                .spyOn(dbiService, 'getFromDBI')
                .mockImplementation(() => Promise.resolve(UNLOCKED_ASSET));
            });

            it('lockModeButtonEnabled should be false', async () => {
              expect(
                await lockModeService.getLockModeObject(ASSET_URI, USERNAME, ROLE_NO_ACCESS),
              ).toStrictEqual({
                username: undefined,
                lockModeButtonEnabled: false,
                isLocked: false,
              });
            });
          });
        });
      });
    });
  });

  describe('test canChangeLockMode', () => {
    describe('asset is unlocked', () => {
      beforeEach(() => {
        jest
          .spyOn(dbiService, 'getFromDBI')
          .mockImplementation(() => Promise.resolve(UNLOCKED_ASSET));
      });

      it('should return true', async () => {
        expect(await lockModeService.canChangeLockMode(ASSET_URI, USERNAME, ROLE_HAS_ACCESS)).toBe(
          true,
        );
      });
    });

    describe('asset is locked', () => {
      describe('locked by other user', () => {
        beforeEach(() => {
          jest
            .spyOn(dbiService, 'getFromDBI')
            .mockImplementation(() => Promise.resolve(LOCKED_BY_OTHER));
        });

        it('should return false', async () => {
          expect(
            await lockModeService.canChangeLockMode(ASSET_URI, USERNAME, ROLE_HAS_ACCESS),
          ).toBe(false);
        });
      });

      describe('locked by requesting user', () => {
        beforeEach(() => {
          jest
            .spyOn(dbiService, 'getFromDBI')
            .mockImplementation(() => Promise.resolve(LOCKED_BY_USERNAME));
        });

        it('should return true', async () => {
          expect(
            await lockModeService.canChangeLockMode(ASSET_URI, USERNAME, ROLE_HAS_ACCESS),
          ).toBe(true);
        });
      });
    });
  });

  describe('test postLockModeUpdate', () => {
    beforeEach(() => {
      jest
        .spyOn(dbiService, 'getFromDBI')
        .mockImplementation(() => Promise.resolve(UNLOCKED_ASSET));
    });

    it('should post locked asset to dbi', async () => {
      const postAssets = jest.spyOn(dbiService, 'postToDBI');

      await lockModeService.postLockModeUpdate(ASSET_URI, LOCKED, USERNAME);

      expect(postAssets).toHaveBeenCalledWith(DBI_URIs.LOCK_MODE_STATE, LOCKED_BY_USERNAME);
    });

    it('should post unlocked asset to dbi', async () => {
      const postAssets = jest.spyOn(dbiService, 'postToDBI');

      await lockModeService.postLockModeUpdate(ASSET_URI, UNLOCKED, USERNAME);

      expect(postAssets).toHaveBeenCalledWith(DBI_URIs.LOCK_MODE_STATE, UNLOCKED_ASSET);
    });
  });
});
