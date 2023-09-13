import { Test, TestingModule } from '@nestjs/testing';
import { FimsWebSocketGuard } from './fims.ws.guard';
import { PermissionsService } from '../../permissions/permissions.service';
import { PERMISSIONS_SERVICE, PermissionLevel } from '../../permissions/interfaces/permission.interface';

describe('FimsWebSocketGuard', () => {
    let guard: FimsWebSocketGuard;
    let permissionsService: PermissionsService;

    beforeEach(async () => {
        const module: TestingModule = await Test.createTestingModule({
            providers: [
                FimsWebSocketGuard,
                {
                    provide: PERMISSIONS_SERVICE,
                    useValue: {
                        ConfirmRoleAccess: jest.fn(),
                    },
                },
            ],
        }).compile();

        guard = module.get<FimsWebSocketGuard>(FimsWebSocketGuard);
        permissionsService = module.get<PermissionsService>(PERMISSIONS_SERVICE);
    });

    it('should be defined', () => {
        expect(guard).toBeDefined();
    });

    describe('test canActivate', () => {
        afterEach(() => {
            jest.clearAllMocks();
        });

        it('should check permissions with data string', () => {
            const confirmRoleAccessSpy = jest.spyOn(permissionsService, 'ConfirmRoleAccess');
            const mockExecutionContext = {
                switchToWs: () => ({
                    getClient: () => ({
                        _socket: {
                            isValid: true,
                            userRole: 'user role',
                            username: 'username',
                        },
                    }),
                    getData: () => ({
                        data: 'data string',
                    }),
                }),
                getClass: () => jest.fn(),
                getHandler: () => jest.fn(),
            };
            guard.canActivate(mockExecutionContext as any);
            expect(confirmRoleAccessSpy).toHaveBeenCalledWith(
                {
                    role: mockExecutionContext.switchToWs().getClient()._socket.userRole,
                    username: mockExecutionContext.switchToWs().getClient()._socket.username,
                },
                PermissionLevel.READ,
                mockExecutionContext.switchToWs().getData().data,
            );
        });

        it('should check permissions with data get object', () => {
            const confirmRoleAccessSpy = jest.spyOn(permissionsService, 'ConfirmRoleAccess');
            const mockExecutionContext = {
                switchToWs: () => ({
                    getClient: () => ({
                        _socket: {
                            isValid: true,
                            userRole: 'user role',
                            username: 'username',
                        },
                    }),
                    getData: () => ({
                        data: {
                            uri: '/example/uri',
                            method: 'get',
                        },
                    }),
                }),
                getClass: () => jest.fn(),
                getHandler: () => jest.fn(),
            };
            guard.canActivate(mockExecutionContext as any);
            expect(confirmRoleAccessSpy).toHaveBeenCalledWith(
                {
                    role: mockExecutionContext.switchToWs().getClient()._socket.userRole,
                    username: mockExecutionContext.switchToWs().getClient()._socket.username,
                },
                PermissionLevel.READ,
                mockExecutionContext.switchToWs().getData().data.uri,
            );
        });

        it('should check permissions with data non-get object', () => {
            const confirmRoleAccessSpy = jest.spyOn(permissionsService, 'ConfirmRoleAccess');
            const mockExecutionContext = {
                switchToWs: () => ({
                    getClient: () => ({
                        _socket: {
                            isValid: true,
                            userRole: 'user role',
                            username: 'username',
                        },
                    }),
                    getData: () => ({
                        data: {
                            uri: '/example/uri',
                            method: 'put',
                        },
                    }),
                }),
                getClass: () => jest.fn(),
                getHandler: () => jest.fn(),
            };
            guard.canActivate(mockExecutionContext as any);
            expect(confirmRoleAccessSpy).toHaveBeenCalledWith(
                {
                    role: mockExecutionContext.switchToWs().getClient()._socket.userRole,
                    username: mockExecutionContext.switchToWs().getClient()._socket.username,
                },
                PermissionLevel.READ_WRITE,
                mockExecutionContext.switchToWs().getData().data.uri,
            );
        });

        it('should return valid', () => {
            jest
                .spyOn(permissionsService, 'ConfirmRoleAccess')
                .mockReturnValue(true);
            const mockExecutionContext = {
                switchToWs: () => ({
                    getClient: () => ({
                        _socket: {
                            isValid: true,
                            userRole: 'user role',
                            username: 'username',
                        },
                    }),
                    getData: () => ({
                        data: 'data string',
                    }),
                }),
                getClass: () => jest.fn(),
                getHandler: () => jest.fn(),
            };
            expect(guard.canActivate(mockExecutionContext as any)).toBeTruthy();
        });

        it('should return invalid from client', () => {
            jest
                .spyOn(permissionsService, 'ConfirmRoleAccess')
                .mockReturnValue(true);
            const mockExecutionContext = {
                switchToWs: () => ({
                    getClient: () => ({
                        _socket: {
                            isValid: false,
                            userRole: 'user role',
                            username: 'username',
                        },
                    }),
                    getData: () => ({
                        data: 'data string',
                    }),
                }),
                getClass: () => jest.fn(),
                getHandler: () => jest.fn(),
            };
            expect(guard.canActivate(mockExecutionContext as any)).toBeFalsy();
        });

        it('should return invalid from confirmRoleAccess', () => {
            jest
                .spyOn(permissionsService, 'ConfirmRoleAccess')
                .mockReturnValue(false);
            const mockExecutionContext = {
                switchToWs: () => ({
                    getClient: () => ({
                        _socket: {
                            isValid: true,
                            userRole: 'user role',
                            username: 'username',
                        },
                    }),
                    getData: () => ({
                        data: 'data string',
                    }),
                }),
                getClass: () => jest.fn(),
                getHandler: () => jest.fn(),
            };
            expect(guard.canActivate(mockExecutionContext as any)).toBeFalsy();
        });
    });
});