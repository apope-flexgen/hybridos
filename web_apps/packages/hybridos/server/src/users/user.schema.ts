import { Prop, Schema, SchemaFactory } from '@nestjs/mongoose';
import { Document } from 'mongoose';
import { Roles } from '../../../shared/types/api/Users/Users.types';

export type UserDocument = User & Document;

export const MIN_LENGTH = 5;
export const MIN_LENGTH_MESSAGE = 'username must be at least 5 characters';
export const MAX_LENGTH = 25;
export const MAX_LENGTH_MESSAGE = 'username must be no more than 25 characters';
export const VALID_USERNAME_REGEX = /^(?!.*__.*)(?!.*\.\..*)[a-z0-9_.]+$/;
export const INVALID_USERNAME_MESSAGE =
  'username may only include letters, numbers, periods, and underscores';

export const VALID_ROLES = [
  Roles.User,
  Roles.Admin,
  Roles.Rest,
  Roles.Developer,
  Roles.Observer,
  Roles.RestReadWrite,
];

export const INVALID_ROLE = `role is invalid, valid roles are [${VALID_ROLES}]`;

@Schema()
export class User {
  @Prop({
    required: true,
    unique: true,
    lowercase: true,
    trim: true,
    minlength: [MIN_LENGTH, MIN_LENGTH_MESSAGE],
    maxlength: [MAX_LENGTH, MAX_LENGTH_MESSAGE],
    validate: (username: string) => {
      if (username.match(VALID_USERNAME_REGEX) === null) {
        throw new Error(INVALID_USERNAME_MESSAGE);
      }
    },
  })
  username: string;
  @Prop({
    required: true,
    enum: { values: VALID_ROLES, message: INVALID_ROLE },
  })
  role: string;
  @Prop({ required: true })
  password: string;
  @Prop({ required: false })
  old_passwords: Array<string>;
  @Prop({ required: false })
  pwdUpdatedDate: Date;
  @Prop({ required: true })
  shared_key: string;
  @Prop({ required: true })
  mfa_enabled: boolean;
  @Prop({ required: true, default: '1.0.0' })
  version: string;
}

export const UserSchema = SchemaFactory.createForClass(User);
