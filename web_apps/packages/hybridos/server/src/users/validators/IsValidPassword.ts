import {
  registerDecorator,
  ValidationOptions,
  ValidatorConstraint,
  ValidatorConstraintInterface,
} from 'class-validator';
import { User } from '../dtos/user.dto';
import { validatePassword } from '../../../../shared/functions/passwordValidation';
import {
  SITE_ADMINS_SERVICE,
  ISiteAdminsService,
} from '../../siteAdmins/interfaces/siteAdmin.service.interface';
import { Inject, Injectable } from '@nestjs/common';

@ValidatorConstraint({ async: true })
@Injectable()
export class ValidPasswordConstraint implements ValidatorConstraintInterface {
  constructor(
    @Inject(SITE_ADMINS_SERVICE) private readonly siteAdminsService: ISiteAdminsService,
  ) {}

  async validate(password: string) {
    const siteAdmins = await this.siteAdminsService.find();

    return validatePassword(password, {
      passwordMinLength: siteAdmins.password.minimum_password_length,
      passwordMaxLength: siteAdmins.password.maximum_password_length,
      lowercase: siteAdmins.password.lowercase,
      uppercase: siteAdmins.password.uppercase,
      digit: siteAdmins.password.digit,
      special: siteAdmins.password.special,
    });
  }

  defaultMessage() {
    return 'password is invalid';
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
    });
  };
}
