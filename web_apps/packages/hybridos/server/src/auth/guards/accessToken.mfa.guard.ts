import { Injectable } from '@nestjs/common'
import { AuthGuard } from '@nestjs/passport'

import { AccessTokenMfaStrategy } from '../strategies/accessToken.mfa.strategy'

@Injectable()
export class AccessTokenMfaGuard extends AuthGuard(AccessTokenMfaStrategy.key) {}
