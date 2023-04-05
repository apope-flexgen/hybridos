import { Injectable } from '@nestjs/common'
import { JwtService } from '@nestjs/jwt'
import { User } from '../../../shared/types/dtos/auth.dto'

@Injectable()
export class SocketAuthService {
    constructor(private readonly jwtService: JwtService) {}
    private readonly socketTokens: Set<string> = new Set()
    addToken(token: string) {
        this.socketTokens.add(token)
    }
    invalidateToken(token: string): boolean {
        return this.socketTokens.delete(token)
    }
    containsToken(token: string): boolean {
        return this.socketTokens.has(token)
    }
    // FIXME: set these back to vars instead of hardcoded admin
    generateToken(user: User): string {
        const payload = {
            role: user.role,
            oneTime: 'fims-socket',
            sub: user.username,
        }
        const token = this.jwtService.sign(payload)
        // const token = crypto.randomBytes(32).toString('hex');
        this.addToken(token)
        return token
    }
    validateToken(token: string): boolean {
        try {
            this.jwtService.verify(token)
        } catch (e) {
            return false
        }

        // validate that it is in the list of valid jwts
        // then remove it from the list of valid jwts
        if (this.containsToken(token)) {
            return this.invalidateToken(token)
        } else {
            return false
        }
    }
}
