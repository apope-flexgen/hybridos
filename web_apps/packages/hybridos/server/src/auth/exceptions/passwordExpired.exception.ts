import { User } from '../../../../shared/types/dtos/auth.dto';

export class PasswordExpiredException extends Error {
  user: User;
  constructor(user: User) {
    super('password is expired');
    this.user = user;
  }
}
