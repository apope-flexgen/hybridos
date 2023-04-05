import { Injectable } from '@nestjs/common'
import { AuthGuard } from '@nestjs/passport'

import { AccessTokenPassExpStrategy } from '../strategies/accessToken.passExp.strategy'

@Injectable()
export class AccessTokenPassExpGuard extends AuthGuard(AccessTokenPassExpStrategy.key) {}
