import {
    isNotEmpty,
    registerDecorator,
    ValidationOptions,
    ValidatorConstraint,
    ValidatorConstraintInterface,
} from 'class-validator'
import { User } from '../dtos/user.dto'
import { INVALID_ROLE, VALID_ROLES } from '../user.schema'

@ValidatorConstraint({ async: true })
export class ValidRoleConstraint implements ValidatorConstraintInterface {
    validate(role: string) {
        return isNotEmpty(VALID_ROLES.find((r) => r == role))
    }
    defaultMessage() {
        return INVALID_ROLE
    }
}

export function IsValidRole(validationOptions?: ValidationOptions): PropertyDecorator {
    return function (object: User, propertyName: string) {
        registerDecorator({
            target: object.constructor,
            propertyName: propertyName,
            options: validationOptions,
            constraints: [],
            validator: ValidRoleConstraint,
        })
    }
}
