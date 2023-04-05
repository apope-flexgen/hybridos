import { PasswordOptions } from '../types/api/Users/Users.types'

export const validatePassword = (password: string, options: PasswordOptions): boolean => {
    if (
        password.length < options.passwordMinLength ||
        password.length > options.passwordMaxLength
    ) {
        return false
    }

    // matches all ascii
    if (!new RegExp(/[!-~]/).test(password)) {
        return false
    }

    // should not match control characters
    if (new RegExp(/[\x00-\x1F\x7F]/).test(password)) {
        return false
    }

    // matches lowercase
    if (options.lowercase && !new RegExp(/[a-z]/).test(password)) {
        return false
    }

    // matches uppercase
    if (options.uppercase && !new RegExp(/[A-Z]/).test(password)) {
        return false
    }

    // matches digits
    if (options.digit && !new RegExp(/[0-9]/).test(password)) {
        return false
    }

    // matches special characters
    if (options.special && !new RegExp(/[!-/:-@[-`{-~]/).test(password)) {
        return false
    }

    return true
}
