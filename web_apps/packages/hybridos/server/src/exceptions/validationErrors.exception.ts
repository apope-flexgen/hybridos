import { ValidationError } from '@nestjs/common';

export class ValidationErrors extends Error {
  errors: ValidationError[];
  constructor(errors: ValidationError[]) {
    super();
    this.errors = errors;
    this.message = this.errors.join('\n');
  }
}
