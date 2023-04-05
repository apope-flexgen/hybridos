import { Injectable } from '@nestjs/common'
import { AuthGuard } from '@nestjs/passport'

import { RadiusStrategy } from '../strategies/radius.strategy'

@Injectable()
export class AuthenticationGuard extends AuthGuard([RadiusStrategy.key, 'local']) {}
