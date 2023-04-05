import { User } from '../../../../shared/types/dtos/auth.dto'

export class MfaRequiredException extends Error {
    user: User
    constructor(user: User) {
        super('mfa is required')
        this.user = user
    }
}
