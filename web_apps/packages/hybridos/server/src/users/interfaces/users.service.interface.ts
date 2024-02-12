import { CreateUserResponse } from 'src/users/responses/createuser.response';
import { User } from '../dtos/user.dto';
import { AllUsersResponse } from '../responses/allusers.response';
import { DeleteUserResponse } from '../responses/deleteuser.response';
import { UserResponse } from '../responses/user.response';

export const USERS_SERVICE = 'UsersService';
export interface IUsersService {
  create(user: User): Promise<CreateUserResponse>;
  delete(id: string): Promise<DeleteUserResponse>;
  readById(id: string): Promise<UserResponse>;
  readByUsername(username: string): Promise<User>;
  all(role: string, currentUserRole?: string): Promise<AllUsersResponse>;
  enableMfa(id: string): Promise<boolean>;
  update(id: string, updateBody: User): Promise<UserResponse>;
}
