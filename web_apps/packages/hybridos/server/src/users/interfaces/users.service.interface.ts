import { User } from '../dtos/user.dto'
import { AllUsersResponse } from '../responses/allusers.response'
import { DeleteUserResponse } from '../responses/deleteuser.response'
import { UserResponse } from '../responses/user.response'

export const USERS_SERVICE = 'UsersService'
export interface IUsersService {
    create(user: User): Promise<UserResponse>
    delete(id: string): Promise<DeleteUserResponse>
    readById(id: string): Promise<UserResponse>
    readByUsername(username: string): Promise<UserResponse>
    all(role: string, currentUserRole?: string): Promise<AllUsersResponse>
    enableMfa(id: string): Promise<boolean>
    update(id: string, updateBody: User): Promise<UserResponse>
}
