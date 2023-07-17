import { INVALID_USERNAME_MESSAGE, MAX_LENGTH, MIN_LENGTH, VALID_ROLES } from "./user.schema";

export const UserDescriptions = {
    allUsersResponse: 'List of HybridOS users.',
    userDeleted: 'Status of delete request.',
    hybridOSId: 'HybridOS user ID.',
    username: `HybridOS username - username must be between ${MIN_LENGTH} and ${MAX_LENGTH} characters; ${INVALID_USERNAME_MESSAGE}`,
    role: `HybridOS user role - valid roles: [${VALID_ROLES}]`,
    password: 'HybridOS user password.',
    old_passwords: 'List of previous user passwords, used for password expiration (when users password expires and certain password settings are set, they may not use new passwords that are in this list).',
    pwdUpdatedDate: 'Date that password was last updated.',
    shared_key: 'MFA shared key.',
    mfa_enabled: 'Status of MFA for user.',
    version: 'Version of User Model.',
}
