export class UserNotFoundException extends Error {
    constructor() {
        super('User Not Found')
    }
}

export class OldPasswordMatchException extends Error {
    constructor() {
        super('repeated password - please choose a new password')
    }
}

export class NegativeOldPasswordCapacityException extends Error {
    constructor() {
        super('capacity cannot be negative')
    }
}
