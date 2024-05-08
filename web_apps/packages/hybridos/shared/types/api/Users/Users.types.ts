export enum Roles {
  User = 'user',
  Admin = 'admin',
  Rest = 'rest',
  Developer = 'developer',
  Observer = 'observer',
  RestReadWrite = 'restreadwrite',
}

export interface PasswordOptions {
  passwordMinLength: number;
  passwordMaxLength: number;
  lowercase: boolean;
  uppercase: boolean;
  digit: boolean;
  special: boolean;
}

export const initialPasswordOptions: PasswordOptions = {
  passwordMinLength: 8,
  passwordMaxLength: 1028,
  lowercase: true,
  uppercase: true,
  digit: true,
  special: true,
}
