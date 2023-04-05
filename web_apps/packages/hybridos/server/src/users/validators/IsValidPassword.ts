import {
    registerDecorator,
    ValidationOptions,
    ValidatorConstraint,
    ValidatorConstraintInterface,
} from 'class-validator'
import { User } from '../dtos/user.dto'
import { validatePassword } from '../../../../shared/functions/passwordValidation'
import {
    APP_SETTINGS_SERVICE,
    IAppSettingsService,
} from '../../appSettings/interfaces/appSetting.service.interface'
import { Inject, Injectable } from '@nestjs/common'

@ValidatorConstraint({ async: true })
@Injectable()
export class ValidPasswordConstraint implements ValidatorConstraintInterface {
    constructor(
        @Inject(APP_SETTINGS_SERVICE) private readonly appSettingsService: IAppSettingsService
    ) {}

    async validate(password: string) {
        const appSettings = await this.appSettingsService.find()

        return validatePassword(password, {
            passwordMinLength: appSettings.password.minimum_password_length,
            passwordMaxLength: appSettings.password.maximum_password_length,
            lowercase: appSettings.password.lowercase,
            uppercase: appSettings.password.uppercase,
            digit: appSettings.password.digit,
            special: appSettings.password.special,
        })
    }

    defaultMessage() {
        return 'password is invalid'
    }
}

export function IsValidPassword(validationOptions?: ValidationOptions): PropertyDecorator {
    return function (object: User, propertyName: string) {
        registerDecorator({
            target: object.constructor,
            propertyName: propertyName,
            options: validationOptions,
            constraints: [],
            validator: ValidPasswordConstraint,
        })
    }
}
