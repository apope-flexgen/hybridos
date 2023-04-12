import { forwardRef, Inject, Injectable, OnApplicationBootstrap } from '@nestjs/common'
import { User } from './dtos/user.dto'
import { IUsersService, USERS_SERVICE } from './interfaces/users.service.interface'
import { UserResponse } from './responses/user.response'
import { IDefaultUserService } from './interfaces/defaultUser.service.interface'

@Injectable()
export class DefaultUserService implements IDefaultUserService, OnApplicationBootstrap {
    constructor(
        @Inject(forwardRef(() => USERS_SERVICE))
        private usersService: IUsersService
    ) {}
    private defaultUser: UserResponse
    async onApplicationBootstrap() {
        const createdUser: UserResponse = await this.createDefaultUser()

        if (createdUser) {
            this.defaultUser = createdUser
            console.log('No users found in database, default user created')
        }
    }
    async createDefaultUser(): Promise<UserResponse> {
        const { users } = await this.usersService.all('')
        console.log('Users found in Database: ', users.length)

        if (users.length > 0) {
            return null
        }

        const defaultUserData: User = {
            username: 'fgdefault',
            role: 'admin',
            password: 'fgdefault1A!',
        }
        const createdUser: UserResponse = await this.usersService.create(defaultUserData)

        return createdUser
    }
    async deleteDefaultUser(): Promise<boolean> {
        // if no defaultUser exists, don't try to delete
        if (!this.defaultUser) {
            return false
        }

        const { userDeleted } = await this.usersService.delete(this.defaultUser.id)
        this.defaultUser = null
        return userDeleted
    }
}
