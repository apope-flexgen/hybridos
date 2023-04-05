import { Roles } from '../api/Users/Users.types'

export class User {
    username?: string
    role:
        | Roles.User
        | Roles.Admin
        | Roles.Rest
        | Roles.Developer
        | Roles.Observer
        | Roles.RestReadWrite
    passwordExpired?: boolean
    mfaRequired?: boolean
    qrCode?: string
    id?: string
    password?: string
    old_passwords?: string[]
    pwdUpdatedDate?: Date
    shared_key?: string
    mfa_enabled?: boolean
    version?: string
}

export class LogoutResponse {
    message: string
}

export class GetUserRequest {
    id: string
}

export class DeleteUserRequest extends GetUserRequest {}

export class UpdateUserRequest extends GetUserRequest {}
