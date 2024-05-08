import { Test, TestingModule } from '@nestjs/testing';
import { FimsHttpPermissionsGuard } from './fims.http.guard';
import {
  PERMISSIONS_SERVICE,
  PermissionLevel,
} from '../../permissions/interfaces/permission.interface';
import { PermissionsService } from '../../permissions/permissions.service';

describe('FimsHttpPermissionsGuard', () => {
  let guard: FimsHttpPermissionsGuard;
  let permissionsService: PermissionsService;

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [
        FimsHttpPermissionsGuard,
        {
          provide: PERMISSIONS_SERVICE,
          useValue: {
            ConfirmRoleAccess: jest.fn(),
          },
        },
      ],
    }).compile();

    guard = module.get<FimsHttpPermissionsGuard>(FimsHttpPermissionsGuard);
    permissionsService = module.get<PermissionsService>(PERMISSIONS_SERVICE);
  });

  it('should be defined', () => {
    expect(guard).toBeDefined();
  });

  describe('test canActivate', () => {
    afterEach(() => {
      jest.clearAllMocks();
    });

    it('should get from url request', async () => {
      const confirmRoleAccessSpy = jest.spyOn(permissionsService, 'ConfirmRoleAccess');
      const mockExecutionContext = {
        switchToHttp: () => ({
          getRequest: () => ({
            user: 'user',
            method: 'GET',
            query: {
              uri: '/query/uri',
            },
            body: {
              uri: '/body/uri',
            },
          }),
          getResponse: () => jest.fn(),
        }),
        getClass: () => jest.fn(),
        getHandler: () => jest.fn(),
      };
      await guard.canActivate(mockExecutionContext as any);
      expect(confirmRoleAccessSpy).toHaveBeenCalledWith(
        mockExecutionContext.switchToHttp().getRequest().user,
        PermissionLevel.READ,
        mockExecutionContext.switchToHttp().getRequest().query.uri,
      );
    });

    it('should get from body', async () => {
      const confirmRoleAccessSpy = jest.spyOn(permissionsService, 'ConfirmRoleAccess');
      const mockExecutionContext = {
        switchToHttp: () => ({
          getRequest: () => ({
            user: 'user',
            method: 'NOTGET',
            query: {
              uri: '/query/uri',
            },
            body: {
              uri: '/body/uri',
            },
          }),
          getResponse: () => jest.fn(),
        }),
        getClass: () => jest.fn(),
        getHandler: () => jest.fn(),
      };
      await guard.canActivate(mockExecutionContext as any);
      expect(confirmRoleAccessSpy).toHaveBeenCalledWith(
        mockExecutionContext.switchToHttp().getRequest().user,
        PermissionLevel.READ_WRITE,
        mockExecutionContext.switchToHttp().getRequest().body.uri,
      );
    });
  });
});
