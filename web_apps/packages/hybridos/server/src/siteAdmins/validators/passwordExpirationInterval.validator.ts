import {
    registerDecorator,
    ValidationOptions,
    ValidatorConstraint,
    ValidatorConstraintInterface,
} from 'class-validator'

import { SiteAdminsDto } from '../dto/create-siteAdmins.dto'

@ValidatorConstraint({ async: true })
export class ValidPasswordExpirationIntervalConstraint implements ValidatorConstraintInterface {
    validate(interval: string): boolean {
        const regexp = new RegExp('^[1-9]\\d*[dmh]$')
        return regexp.test(interval)
    }
    defaultMessage() {
        return 'Invalid password expiration interval, interval must be a positive integer followed by one of [d, m, h]'
    }
}

export function ValidPassExpInterval(validationOptions?: ValidationOptions): PropertyDecorator {
    return function (object: SiteAdminsDto, propertyName: string) {
        registerDecorator({
            target: object.constructor,
            propertyName: propertyName,
            options: validationOptions,
            constraints: [],
            validator: ValidPasswordExpirationIntervalConstraint,
        })
    }
}
