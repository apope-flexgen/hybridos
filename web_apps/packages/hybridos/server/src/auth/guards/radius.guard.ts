import { Injectable } from '@nestjs/common';
import { AuthGuard } from '@nestjs/passport';

import { RadiusStrategy } from '../strategies/radius.strategy';

@Injectable()
export class RadiusGuard extends AuthGuard(RadiusStrategy.key) {}
