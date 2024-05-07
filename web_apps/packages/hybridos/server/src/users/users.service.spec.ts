import { getModelToken } from '@nestjs/mongoose';
import { Test, TestingModule } from '@nestjs/testing';
import { DeleteResult } from 'mongodb';
import { Model } from 'mongoose';
import { SiteAdminsService } from '../siteAdmins/siteAdmins.service';
import { SITE_ADMINS_SERVICE } from '../siteAdmins/interfaces/siteAdmin.service.interface';
import { SiteAdminDocument } from '../siteAdmins/schemas/siteAdmins.schema';
import { DefaultUserService } from './defaultUser.service';
import { UserNotFoundException } from './exceptions/exceptions';
import { DEFAULT_USER_SERVICE } from './interfaces/defaultUser.service.interface';
import { IUsersService, USERS_SERVICE } from './interfaces/users.service.interface';
import { UserResponse } from './responses/user.response';
import { User, UserDocument } from './user.schema';
import { UsersService } from './users.service';

const mockUser = (id = '631118fdec0291252e0fba15'): UserResponse => ({
  id,
});

describe('UsersService', () => {
  let usersService: IUsersService;
  let userModel: Model<UserDocument>;
  let siteAdminModel: Model<SiteAdminDocument>;
  const defaultSiteAdmins = {
    password: {
      multi_factor_authentication: false,
      password_expiration: false,
      minimum_password_length: 8,
      maximum_password_length: 128,
      password_expiration_interval: '8d',
      old_passwords: 0,
      password_regular_expression: JSON.stringify(
        /^(?=.*[a-z])(?=.*[A-Z])(?=.*[0-9])(?=.*[!"#\$%&'\*\+,\.\/:;=\\?@\^`\|~])/,
      ),
    },
    radius: {
      is_enabled: false,
      ip_address: '127.0.0.1',
      port: '1812',
      secret_phrase: 'testing123',
      wait_time: 5000,
      is_local_auth_disabled: false,
    },
  };

  beforeEach(async () => {
    const module: TestingModule = await Test.createTestingModule({
      providers: [
        UsersService,
        {
          provide: getModelToken(User.name),
          useValue: {
            save: jest.fn(),
            new: jest.fn().mockResolvedValue(mockUser()),
            constructor: jest.fn().mockResolvedValue(mockUser()),
            create: jest.fn(),
            deleteOne: jest.fn(),
            findById: jest.fn(),
            find: jest.fn(),
            findOneAndUpdate: jest.fn(),
            findOne: jest.fn(),
          },
        },
        {
          provide: SITE_ADMINS_SERVICE,
          useClass: SiteAdminsService,
        },
        {
          provide: getModelToken('siteadmins'),
          useValue: {
            findOne: jest.fn(),
            create: jest.fn(),
          },
        },
        {
          provide: DEFAULT_USER_SERVICE,
          useClass: DefaultUserService,
        },
        {
          useClass: UsersService,
          provide: USERS_SERVICE,
        },
      ],
    }).compile();

    usersService = module.get<UsersService>(UsersService);
    userModel = module.get<Model<UserDocument>>(getModelToken(User.name));
    siteAdminModel = module.get<Model<SiteAdminDocument>>(getModelToken('siteadmins'));
  });

  describe('create', () => {
    it('should create a new user', async () => {
      const newUser = mockUser('631118fdec0291252e0fba15');
      jest.spyOn(userModel, 'create').mockImplementationOnce(() =>
        Promise.resolve({
          _id: '631118fdec0291252e0fba15',
          username: 'flex_admin',
          role: 'admin',
        }),
      );
      const res = await usersService.create({
        username: 'flex_admin',
        role: 'admin',
        password: '',
        old_passwords: [],
        pwdUpdatedDate: undefined,
        shared_key: '',
        mfa_enabled: false,
        version: '',
      });
      expect(res).toEqual(newUser);
    });
  });

  describe('update', () => {
    it('should update user password', async () => {
      const update = {
        role: 'admin',
        password: '7ujm&UJM',
        username: 'flexgen_user',
      };

      const testSiteAdmins = {
        password: {
          password_expiration: false,
          minimum_password_length: 8,
          maximum_password_length: 128,
          password_expiration_interval: '8d',
          old_passwords: 0,
          password_regular_expression: JSON.stringify(
            /^(?=.*[a-z])(?=.*[A-Z])(?=.*[0-9])(?=.*[!"#\$%&'\*\+,\.\/:;=\\?@\^`\|~])/,
          ),
          multi_factor_authentication: false,
        },
        radius: {
          is_enabled: false,
          ip_address: '127.0.0.1',
          port: '1812',
          secret_phrase: 'testing123',
          wait_time: 5000,
          is_local_auth_disabled: false,
        },
      };

      const existingUser = {
        _id: '631118fdec0291252e0fba15',
        username: 'flexgen_user',
        role: 'admin',
        password: '',
        old_passwords: ['hello', 'hello2'],
        pwdUpdatedDate: new Date(Date.now()),
        shared_key: '',
        mfa_enabled: false,
        version: '',
      };

      const updatedRecord = {
        _id: '631118fdec0291252e0fba15',
        username: 'flexgen_user',
        role: 'admin',
      };

      siteAdminModel.findOne = jest.fn().mockImplementation(() => ({
        exec: jest.fn().mockResolvedValueOnce(testSiteAdmins),
      }));

      jest.spyOn(userModel, 'findById').mockResolvedValue(existingUser);

      userModel.findOneAndUpdate = jest.fn().mockImplementation(() => ({
        exec: jest.fn().mockResolvedValue(updatedRecord),
      }));

      const res = await usersService.update('631118fdec0291252e0fba15', update);
      const { _id, ...user } = updatedRecord;
      expect(res).toEqual({ id: _id, ...user });
    });

    it('should update role', async () => {
      const update = {
        username: 'flexgen_user',
        role: 'developer',
        password: '7ujm&UJM',
      };

      const existingUser = {
        _id: '631118fdec0291252e0fba15',
        username: 'flexgen_user',
        role: 'admin',
        password: '',
        old_passwords: [],
        pwdUpdatedDate: new Date(Date.now()),
        shared_key: '',
        mfa_enabled: false,
        version: '',
      };

      const updatedRecord = {
        _id: '631118fdec0291252e0fba15',
        username: 'flexgen_user',
        role: update.role,
      };

      siteAdminModel.findOne = jest.fn().mockImplementation(() => ({
        exec: jest.fn().mockResolvedValueOnce(defaultSiteAdmins),
      }));

      jest.spyOn(userModel, 'findById').mockResolvedValue(existingUser);

      userModel.findOneAndUpdate = jest.fn().mockImplementation(() => ({
        exec: jest.fn().mockResolvedValue(updatedRecord),
      }));

      const res = await usersService.update('631118fdec0291252e0fba15', update);
      const { _id, ...user } = updatedRecord;
      expect(res).toEqual({ id: _id, ...user });
    });

    it('should fail to update user: user not found', async () => {
      const update = {
        username: 'flexgen_user',
        role: 'developer',
        password: '7ujm&UJM',
      };

      const updatedRecord = {
        _id: '631118fdec0291252e0fba15',
        username: 'flexgen_user',
        role: update.role,
        password: '',
        old_passwords: [],
        shared_key: '',
        mfa_enabled: false,
        version: '',
      };

      siteAdminModel.findOne = jest.fn().mockImplementation(() => ({
        exec: jest.fn().mockResolvedValueOnce(defaultSiteAdmins),
      }));

      jest.spyOn(userModel, 'findById').mockResolvedValue(null);

      userModel.findOneAndUpdate = jest.fn().mockImplementation(() => ({
        exec: jest.fn().mockResolvedValue(updatedRecord),
      }));
      await expect(usersService.update('631118fdec0291252e0fba15', update)).rejects.toThrow(
        UserNotFoundException,
      );
    });

    it('should fail to update user: unknown exception', async () => {
      const update = {
        username: 'flexgen_user',
        role: 'developer',
        password: '7ujm&UJM',
      };
      const error = Error('failure');

      const existingUser = {
        _id: '631118fdec0291252e0fba15',
        username: 'flexgen_user',
        role: 'admin',
        password: '',
        old_passwords: [],
        pwdUpdatedDate: new Date(Date.now()),
        shared_key: '',
        mfa_enabled: false,
        version: '',
      };

      siteAdminModel.findOne = jest.fn().mockImplementation(() => ({
        exec: jest.fn().mockResolvedValueOnce(defaultSiteAdmins),
      }));

      jest.spyOn(userModel, 'findById').mockResolvedValue(existingUser);

      userModel.findOneAndUpdate = jest.fn().mockImplementation(() => ({
        exec: jest.fn().mockImplementation(() => {
          throw error;
        }),
      }));
      await expect(usersService.update('631118fdec0291252e0fba15', update)).rejects.toThrow(error);
    });
  });

  describe('delete', () => {
    it('should delete a user', async () => {
      const TEST_ID = '1';
      jest.spyOn(userModel, 'deleteOne').mockResolvedValue({
        deletedCount: 1,
      } as DeleteResult);
      const res = await usersService.delete(TEST_ID);
      expect(res).toEqual({ userDeleted: true });
    });

    it('should fail to delete: user not found', async () => {
      const TEST_ID = '1';
      jest.spyOn(userModel, 'deleteOne').mockResolvedValue({
        deletedCount: 0,
      } as DeleteResult);
      await expect(usersService.delete(TEST_ID)).rejects.toThrow(UserNotFoundException);
    });

    it('should fail to delete a user because of an unknown exception', async () => {
      const TEST_ID = '1';
      const ERROR = new Error('Error!!');
      jest.spyOn(userModel, 'deleteOne').mockImplementation(() => {
        throw ERROR;
      });
      await expect(usersService.delete(TEST_ID)).rejects.toThrow(ERROR);
    });
  });

  describe('read', () => {
    it('should read a user by id', async () => {
      const TEST_ID = '1';
      const result = {
        _id: 12,
        username: 'flexgen_user',
        role: 'admin',
      };
      jest.spyOn(userModel, 'findById').mockResolvedValue(result);
      const res = await usersService.readById(TEST_ID);
      const { _id, ...user } = result;
      expect(res).toEqual({ id: _id, ...user });
    });

    it('should fail to read by id: user not found', async () => {
      const TEST_ID = '1';
      const NOT_FOUND_VALUE = null;
      jest.spyOn(userModel, 'findById').mockResolvedValue(NOT_FOUND_VALUE);
      await expect(usersService.readById(TEST_ID)).rejects.toThrow(UserNotFoundException);
    });

    it('should fail to read a user by id because of an unknown exception', async () => {
      const TEST_ID = '1';
      const ERROR = new Error('Error!!');
      jest.spyOn(userModel, 'findById').mockImplementation(() => {
        throw ERROR;
      });
      await expect(usersService.readById(TEST_ID)).rejects.toThrow(ERROR);
    });

    it('should read a user by username', async () => {
      const TEST_USERNAME = 'flexgen_user';
      const result = {
        _id: 12,
        username: 'flexgen_user',
        role: 'admin',
        password: '',
        old_passwords: [],
        pwdUpdatedDate: Date.now(),
        shared_key: '',
        mfa_enabled: false,
        version: '',
      };
      jest.spyOn(userModel, 'findOne').mockResolvedValue(result);
      const res = await usersService.readByUsername(TEST_USERNAME);
      const { _id, ...user } = result;
      expect(res).toEqual({ id: _id, ...user });
    });

    it('should fail to read: username not found', async () => {
      const TEST_USERNAME = 'flex_user';
      const NOT_FOUND_VALUE = null;
      jest.spyOn(userModel, 'findOne').mockResolvedValue(NOT_FOUND_VALUE);
      await expect(usersService.readById(TEST_USERNAME)).rejects.toThrow(UserNotFoundException);
    });

    it('should fail to read a user by username because of an unknown exception', async () => {
      const TEST_USERNAME = 'flexgen_user';
      const ERROR = new Error('Error!!');
      jest.spyOn(userModel, 'findOne').mockImplementation(() => {
        throw ERROR;
      });
      await expect(usersService.readByUsername(TEST_USERNAME)).rejects.toThrow(ERROR);
    });
  });

  describe('read all', () => {
    it('should read a user', async () => {
      const TEST_ROLE = 'admin';
      const queryResult = [
        {
          _id: 12,
          username: 'flexgen_user',
          role: 'admin',
          password: '',
          old_passwords: [],
          pwdUpdatedDate: Date.now(),
          shared_key: '',
          mfa_enabled: false,
          version: '',
        },
      ];
      const returnResult = {
        users: [
          {
            id: 12,
            username: 'flexgen_user',
            role: 'admin',
          },
        ],
      };
      jest.spyOn(userModel, 'find').mockResolvedValue(queryResult);
      const res = await usersService.all(TEST_ROLE);
      expect(res).toEqual(returnResult);
    });

    it('should fail to read all users because of an unknown exception', async () => {
      const TEST_ROLE = 'admin';
      const ERROR = new Error('Error!!');
      jest.spyOn(userModel, 'find').mockImplementation(() => {
        throw ERROR;
      });
      await expect(usersService.all(TEST_ROLE)).rejects.toThrow(ERROR);
    });
  });
});
