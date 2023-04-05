import { PickType } from '@nestjs/swagger'

import { ApiLoginResponse } from './login.response'

export class ApiLoginPassExpResponse extends PickType(ApiLoginResponse, [
    'username',
    'role',
    'passwordExpired',
] as const) {}
