import { Injectable } from '@nestjs/common';
import { AuthGuard } from '@nestjs/passport';

import { TotpStrategy } from '../strategies/totp.strategy';

@Injectable()
export class TotpGuard extends AuthGuard(TotpStrategy.key) {}
