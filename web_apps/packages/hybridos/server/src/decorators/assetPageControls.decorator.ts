import { createParamDecorator, ExecutionContext } from '@nestjs/common'
import { Roles } from 'shared/types/api/Users/Users.types'

export const VALID_ASSET_CONTROL_ROLES = new Map<Roles, boolean>([
    [Roles.Developer, true],
    [Roles.Admin, true],
    [Roles.User, true],
    [Roles.RestReadWrite, false],
    [Roles.Observer, false],
    [Roles.Rest, false],
])

export const EnableAssetPageControls = createParamDecorator(
    (_data: string, ctx: ExecutionContext) => {
        const client = ctx.switchToWs().getClient()
        const socketRole = client._socket.userRole as Roles
        const hasAccess = VALID_ASSET_CONTROL_ROLES.get(socketRole)
        return hasAccess
    }
)
