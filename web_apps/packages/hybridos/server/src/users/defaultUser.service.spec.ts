/* eslint-disable max-nested-callbacks */
import { Test, TestingModule } from '@nestjs/testing';
import { DefaultUserService } from './defaultUser.service';
import { User } from './dtos/user.dto';
import { IDefaultUserService } from './interfaces/defaultUser.service.interface';
import { IUsersService, USERS_SERVICE } from './interfaces/users.service.interface';
import { DeleteUserResponse } from './responses/deleteuser.response';
import { UserResponse } from './responses/user.response';

describe('DefaultUserService', () => {
  let defaultUserService: IDefaultUserService;
  let usersService: IUsersService;

  const user: UserResponse = {
    id: '1234',
  };

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [
        DefaultUserService,
        {
          provide: USERS_SERVICE,
          useValue: {
            all: jest.fn(),
            create: jest.fn(),
            delete: jest.fn(),
          },
        },
      ],
    }).compile();

    defaultUserService = module.get<DefaultUserService>(DefaultUserService);
    usersService = module.get<IUsersService>(USERS_SERVICE);
  });

  describe('test create default user', () => {
    describe('no users in the database', () => {
      beforeEach(() => {
        jest.spyOn(usersService, 'all').mockResolvedValue({ users: [] });
      });

      it('should successfully create default user', async () => {
        jest.spyOn(usersService, 'create').mockResolvedValue(user);

        expect(await defaultUserService.createDefaultUser()).toBeTruthy();
      });

      it('should throw an error when an error occurs when creating user', async () => {
        jest
          .spyOn(usersService, 'create')
          .mockImplementation(async (userDto: User): Promise<UserResponse> => {
            throw new Error('error when creating user');
          });

        await expect(defaultUserService.createDefaultUser).rejects.toThrowError();
      });
    });

    describe('users already exist in the database', () => {
      beforeEach(() => {
        jest.spyOn(usersService, 'all').mockResolvedValue({ users: [user] });
      });

      it('should not create a default user', async () => {
        expect(await defaultUserService.createDefaultUser()).toBeFalsy();
      });
    });
  });

  describe('test delete default user', () => {
    describe('no default user exists', () => {
      it('should return false when no default user exists', async () => {
        expect(await defaultUserService.deleteDefaultUser()).toBe(false);
      });
    });

    describe('default user does exist', () => {
      beforeEach(async () => {
        jest.spyOn(usersService, 'all').mockResolvedValue({ users: [] });
        jest.spyOn(usersService, 'create').mockResolvedValue(user);

        await defaultUserService.onApplicationBootstrap();
      });

      it('should return true when default user is deleted', async () => {
        jest.spyOn(usersService, 'delete').mockResolvedValue({ userDeleted: true });

        expect(await defaultUserService.deleteDefaultUser()).toBe(true);
      });

      it('should throw an error when error occurs while deleting user', async () => {
        jest
          .spyOn(usersService, 'delete')
          .mockImplementation(async (id: string): Promise<DeleteUserResponse> => {
            throw new Error('error when deleting user');
          });

        await expect(defaultUserService.createDefaultUser).rejects.toThrowError();
      });
    });
  });
});
