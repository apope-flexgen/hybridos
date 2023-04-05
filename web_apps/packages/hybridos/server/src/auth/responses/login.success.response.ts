import { PickType } from '@nestjs/swagger'

import { ApiLoginResponse } from './login.response'

export class LoginSuccessResponse extends PickType(ApiLoginResponse, [
    'username',
    'role',
    'accessToken',
] as const) {}
