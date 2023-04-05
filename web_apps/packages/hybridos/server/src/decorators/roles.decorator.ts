import { SetMetadata } from '@nestjs/common'

export const ApprovedRoles = (...roles: string[]) => SetMetadata('roles', roles)
