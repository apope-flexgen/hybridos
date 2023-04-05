export enum Roles {
    User = 'user',
    Admin = 'admin',
    Rest = 'rest',
    Developer = 'developer',
    Observer = 'observer',
    RestReadWrite = 'restreadwrite',
}

export interface PasswordOptions {
    passwordMinLength: number
    passwordMaxLength: number
    lowercase: boolean
    uppercase: boolean
    digit: boolean
    special: boolean
}
