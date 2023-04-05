import { createParamDecorator, ExecutionContext } from "@nestjs/common";
import { User } from "src/users/dtos/user.dto";

export const UserFromSocket = createParamDecorator(
    (_data: string, ctx: ExecutionContext): User => {
        const client = ctx.switchToWs().getClient()
        const user = {
            username: client._socket.username,
            role: client._socket.userRole,
        }
        return user
    },
);
