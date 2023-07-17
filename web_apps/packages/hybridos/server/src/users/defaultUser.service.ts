import { forwardRef, Inject, Injectable, OnApplicationBootstrap } from '@nestjs/common';
import { User } from './dtos/user.dto';
import { IUsersService, USERS_SERVICE } from './interfaces/users.service.interface';
import { UserResponse } from './responses/user.response';
import { IDefaultUserService } from './interfaces/defaultUser.service.interface';

@Injectable()
export class DefaultUserService implements IDefaultUserService, OnApplicationBootstrap {
  private defaultUser: UserResponse;

  constructor(
    @Inject(forwardRef(() => USERS_SERVICE))
    private usersService: IUsersService,
  ) {}

  async onApplicationBootstrap() {
    const createdUser: UserResponse = await this.createDefaultUser();

    if (createdUser) {
      this.setDefaultUser(createdUser);
      console.log('No users found in database, default user created');
    }
  }

  async createDefaultUser(): Promise<UserResponse> {
    const { users } = await this.usersService.all('');
    console.log('Users found in Database: ', users.length);

    if (users.length > 0) {
      return null;
    }

    const defaultUserData: User = {
      username: 'fgdefault',
      role: 'admin',
      password: 'fgdefault1A!',
    };
    const createdUser: UserResponse = await this.usersService.create(defaultUserData);

    return createdUser;
  }

  async deleteDefaultUser(): Promise<boolean> {
    // if no defaultUser exists, don't try to delete
    if (!this.getDefaultUser()) {
      return false;
    }

    try {
      const { userDeleted } = await this.usersService.delete(this.getDefaultUserID());
      this.setDefaultUser(null);
      return userDeleted;
    } catch (e) {
      throw new Error('Error deleting default user');
    }
  }

  getDefaultUser = (): UserResponse | null => this.defaultUser;

  setDefaultUser = (defaultUser: UserResponse | null) => {
    this.defaultUser = defaultUser;
  };

  getDefaultUserID = (): string => this.getDefaultUser()?.id.toString() ?? '';
}
