import {
    registerDecorator,
    ValidationOptions,
    ValidatorConstraint,
    ValidatorConstraintInterface,
    isEmpty,
} from 'class-validator'
import { User } from '../dtos/user.dto'
import { MIN_LENGTH, MAX_LENGTH, VALID_USERNAME_REGEX } from '../user.schema'

export const INVALID_USERNAME_API = `username may only include letters,
  numbers, periods, and underscores and have a length of ${MIN_LENGTH} to ${MAX_LENGTH} characters.`

@ValidatorConstraint({ async: true })
export class ValidUsernameConstraint implements ValidatorConstraintInterface {
    validate(username: string) {
        if (isEmpty(username)) {
            return false
        }

        const lowercaseTrimmed = username.toLowerCase().trim()
        if (lowercaseTrimmed.length < MIN_LENGTH || lowercaseTrimmed.length > MAX_LENGTH) {
            return false
        }

        const validChars = new RegExp(VALID_USERNAME_REGEX)
        const validUsername = validChars.test(lowercaseTrimmed)

        return validUsername
    }
    defaultMessage() {
        return INVALID_USERNAME_API
    }
}

export function IsValidUsername(validationOptions?: ValidationOptions): PropertyDecorator {
    return function (object: User, propertyName: string) {
        registerDecorator({
            target: object.constructor,
            propertyName: propertyName,
            options: validationOptions,
            constraints: [],
            validator: ValidUsernameConstraint,
        })
    }
}
