// TODO: refactor file so we can remove this eslint disable
/* eslint-disable max-statements */
/* eslint-disable max-lines */
import { forwardRef, Inject, Injectable } from '@nestjs/common';
import { InjectModel } from '@nestjs/mongoose';
import { compare, hash } from 'bcryptjs';
import { isNotEmpty } from 'class-validator';
import crypto from 'crypto';
import mongoose, { Model } from 'mongoose';
import { authenticator } from 'otplib';
import { Roles } from '../../../shared/types/api/Users/Users.types';
import {
  SITE_ADMINS_SERVICE,
  ISiteAdminsService,
} from '../siteAdmins/interfaces/siteAdmin.service.interface';
import { User as UserDto } from './dtos/user.dto';
import {
  NegativeOldPasswordCapacityException,
  OldPasswordMatchException,
  UserNotFoundException,
} from './exceptions/exceptions';
import {
  DEFAULT_USER_SERVICE,
  IDefaultUserService,
} from './interfaces/defaultUser.service.interface';
import { IUsersService } from './interfaces/users.service.interface';
import { AllUsersResponse } from './responses/allusers.response';
import { DeleteUserResponse } from './responses/deleteuser.response';
import { UserResponse } from './responses/user.response';
import { User, UserDocument } from './user.schema';
import { CreateUserResponse } from 'src/users/responses/createuser.response';

@Injectable()
export class UsersService implements IUsersService {
  constructor(
    @InjectModel(User.name) private readonly userModel: Model<UserDocument>,
    @Inject(SITE_ADMINS_SERVICE) private readonly siteAdminsService: ISiteAdminsService,
    @Inject(forwardRef(() => DEFAULT_USER_SERVICE))
    private readonly defaultUserService: IDefaultUserService,
  ) {}

  public async create(userDto: UserDto): Promise<CreateUserResponse> {
    userDto = await this.hydrateUser(userDto);
    const newUser = await this.userModel.create(userDto);

    // attempt to delete the default user
    await this.defaultUserService.deleteDefaultUser();

    return {
      id: newUser._id,
    };
  }

  private async hydrateUser(userDto: UserDto): Promise<UserDto> {
    if (userDto?.password) {
      const HASH_COST = 10;
      userDto.password = await hash(userDto.password, HASH_COST);
      userDto.old_passwords = new Array<string>(userDto.password);

      userDto.pwdUpdatedDate = new Date(Date.now());
    }

    authenticator.options = { crypto };
    userDto.shared_key = authenticator.generateSecret();
    userDto.mfa_enabled = false;

    // TODO: Do not hardcode.
    userDto.version = '1.0.0';

    return userDto;
  }

  public async delete(id: string): Promise<DeleteUserResponse> {
    const result = await this.userModel.deleteOne({ _id: id });

    if (result.deletedCount == 0) {
      throw new UserNotFoundException();
    }

    if (id === this.defaultUserService.getDefaultUserID()) {
      this.defaultUserService.setDefaultUser(null);
    }

    return {
      userDeleted: true,
    };
  }

  public async readById(id: string): Promise<UserResponse> {
    const result = await this.userModel.findById(id);

    if (result == null) {
      throw new UserNotFoundException();
    }

    return {
      id: result._id,
      username: result.username,
      role: result.role as
        | Roles.User
        | Roles.Admin
        | Roles.Rest
        | Roles.Developer
        | Roles.Observer
        | Roles.RestReadWrite,
    };
  }

  public async readByUsername(username: string): Promise<UserDto> {
    const result = await this.userModel.findOne({ username: username });

    if (result == null) {
      throw new UserNotFoundException();
    }

    return {
      id: result._id,
      username: result.username,
      role: result.role as
        | Roles.User
        | Roles.Admin
        | Roles.Rest
        | Roles.Developer
        | Roles.Observer
        | Roles.RestReadWrite,
      password: result.password,
      old_passwords: result.old_passwords,
      pwdUpdatedDate: result.pwdUpdatedDate,
      shared_key: result.shared_key,
      mfa_enabled: result.mfa_enabled,
      version: result.version,
    };
  }

  public async all(
    role:
      | Roles.User
      | Roles.Admin
      | Roles.Rest
      | Roles.Developer
      | Roles.Observer
      | Roles.RestReadWrite,
    currentUserRole?: string,
  ): Promise<AllUsersResponse> {
    let filter = {};
    const users = new Array<UserResponse>();

    if (isNotEmpty(role)) {
      filter['role'] = role;
    } else if (currentUserRole == Roles.Developer) {
      filter = {
        role: { $ne: Roles.RestReadWrite },
      };
    } else {
      filter = {
        $and: [{ role: { $ne: Roles.RestReadWrite } }, { role: { $ne: Roles.Developer } }],
      };
    }

    const result = await this.userModel.find(filter);

    result.forEach((result) => {
      users.push({
        id: result._id,
        username: result.username,
        role: result.role as
          | Roles.User
          | Roles.Admin
          | Roles.Rest
          | Roles.Developer
          | Roles.Observer
          | Roles.RestReadWrite,
      });
    });

    return {
      users: users,
    };
  }

  public async enableMfa(id: string): Promise<boolean> {
    const update = { mfa_enabled: true };

    const result = await this.userModel.findOneAndUpdate(
      { _id: id },
      { $set: update },
      { new: true },
    );

    return result.mfa_enabled;
  }

  public async update(id: string, updateBody: UserDto): Promise<UserResponse> {
    const siteAdmins = await this.siteAdminsService.find();
    const existingUser = await this.userModel.findById(id);

    if (existingUser == null) {
      throw new UserNotFoundException();
    }

    const update = await this.buildUserUpdate(
      existingUser,
      updateBody,
      siteAdmins.password.old_passwords,
    );

    const result = await this.userModel
      .findOneAndUpdate(
        { _id: id },
        {
          $set: update,
        },
        { new: true },
      )
      .exec();

    return {
      id: result._id,
      username: result.username,
      role: result.role as
        | Roles.User
        | Roles.Admin
        | Roles.Rest
        | Roles.Developer
        | Roles.Observer
        | Roles.RestReadWrite,
    };
  }

  private async buildUserUpdate(
    existingUser: UserDocument,
    updateBody: UserDto,
    oldPasswordCapacity: number,
  ): Promise<object> {
    let newUser = new UserDto();

    newUser.username = updateBody?.username;
    newUser.password = updateBody?.password;
    newUser.role = updateBody?.role as
      | Roles.User
      | Roles.Admin
      | Roles.Rest
      | Roles.Developer
      | Roles.Observer
      | Roles.RestReadWrite;

    newUser = await this.hydrateUser(newUser);

    const updateFields = {};
    if (updateBody?.username) updateFields['username'] = newUser.username;
    if (updateBody?.role) updateFields['role'] = newUser.role;

    if (updateBody?.password) {
      // TODO: Add compare to auth service.
      const passwordsMatch = await compare(updateBody.password, existingUser.password);

      if (!passwordsMatch) {
        for (const oldPassword of existingUser.old_passwords) {
          // TODO: Add compare to auth service.
          const match = await compare(updateBody.password, oldPassword);

          if (match) {
            throw new OldPasswordMatchException();
          }
        }

        updateFields['password'] = newUser.password;
        updateFields['pwdUpdatedDate'] = new Date(Date.now());

        if (oldPasswordCapacity > 0) {
          existingUser.old_passwords = this.truncateOldPasswords(
            existingUser.old_passwords,
            oldPasswordCapacity - 1,
          );
          existingUser.old_passwords.push(newUser.password);
          updateFields['old_passwords'] = existingUser.old_passwords;
        } else {
          updateFields['old_passwords'] = new Array<string>();
        }
      }
    }

    return updateFields;
  }

  private truncateOldPasswords(oldPasswords: Array<string>, capacity: number): string[] {
    if (capacity < 0) {
      throw new NegativeOldPasswordCapacityException();
    }

    let temp = oldPasswords;

    if (capacity == 0) {
      return [];
    } else {
      while (temp.length > capacity) {
        temp = temp.slice(1);
      }
    }
    return temp;
  }
}
