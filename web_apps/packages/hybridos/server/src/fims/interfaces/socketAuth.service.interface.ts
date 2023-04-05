import { User } from '../../../../shared/types/dtos/auth.dto'

export const SOCKET_AUTH_SERVICE = 'SocketAuthService'

export interface ISocketAuthService {
    addToken(token: string)
    invalidateToken(token: string): boolean
    containsToken(token: string): boolean
    generateToken(user: User): string
    validateToken(token: string): boolean
}
