import { ForbiddenException } from '@nestjs/common';

export class InvalidURIPermissions extends ForbiddenException {
  constructor() {
    super('Invalid URI Permissions');
  }
}
